/**
 * limbo_string_names.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "limbo_string_names.h"
#include "godot_cpp/variant/string_name.hpp"

#ifdef LIMBOAI_MODULE
#define SN(m_arg) (StaticCString::create(m_arg))
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#define SN(m_arg) (StringName(m_arg))
#endif // LIMBOAI_GDEXTENSION

LimboStringNames *LimboStringNames::singleton = nullptr;

LimboStringNames::LimboStringNames() {
	_generate_name = SN("_generate_name");
	_setup = SN("_setup");
	_enter = SN("_enter");
	_exit = SN("_exit");
	_tick = SN("_tick");
	behavior_tree_finished = SN("behavior_tree_finished");
	setup = SN("setup");
	entered = SN("entered");
	exited = SN("exited");
	updated = SN("updated");
	_update = SN("_update");
	state_changed = SN("state_changed");
	_get_configuration_warning = SN("_get_configuration_warning");
	changed = SN("changed");
	changed = SN("emit_changed");
	_weight_ = SN("_weight_");
	error_value = SN("error_value");
	behavior_tree = SN("behavior_tree");

	EVENT_FINISHED = "finished";
	repeat_forever.parse_utf8("Repeat ∞");
}