/**
 * bt_comment.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_comment.h"

Ref<BTTask> BTComment::clone() const {
	if (Engine::get_singleton()->is_editor_hint()) {
		return BTTask::clone();
	}
	return nullptr;
}
