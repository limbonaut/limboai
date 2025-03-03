/**
 * bt_comment.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_comment.h"

#include "bt_task.h"

#ifdef LIMBOAI_MODULE
#include "core/config/engine.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#endif // LIMBOAI_GDEXTENSION

void BTComment::set_enabled(bool p_enabled) {
	// BTComment is always disabled.
}

Ref<BTTask> BTComment::clone() const {
	if (Engine::get_singleton()->is_editor_hint()) {
		return BTTask::clone();
	}
	return nullptr;
}

PackedStringArray BTComment::get_configuration_warnings() {
	PackedStringArray warnings = BTTask::get_configuration_warnings();
	if (get_enabled_child_count() > 0) {
		warnings.append("Can only have other comment tasks as children.");
	}
	if (get_parent().is_null()) {
		warnings.append("Can't be the root task.");
	}
	return warnings;
}

BTComment::BTComment() {
	_set_enabled(false);
}
