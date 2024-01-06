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

LimboStringNames *LimboStringNames::singleton = nullptr;

LimboStringNames::LimboStringNames() {
	_generate_name = StringName("_generate_name");
	_setup = StringName("_setup");
	_enter = StringName("_enter");
	_exit = StringName("_exit");
	_tick = StringName("_tick");
	behavior_tree_finished = StringName("behavior_tree_finished");
	setup = StringName("setup");
	entered = StringName("entered");
	exited = StringName("exited");
	updated = StringName("updated");
	_update = StringName("_update");
	state_changed = StringName("state_changed");
	_get_configuration_warning = StringName("_get_configuration_warning");
}