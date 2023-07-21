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

#include "core/string/string_name.h"

LimboStringNames *LimboStringNames::singleton = nullptr;

LimboStringNames::LimboStringNames() {
	_generate_name = StaticCString::create("_generate_name");
	_setup = StaticCString::create("_setup");
	_enter = StaticCString::create("_enter");
	_exit = StaticCString::create("_exit");
	_tick = StaticCString::create("_tick");
	behavior_tree_finished = StaticCString::create("behavior_tree_finished");
	setup = StaticCString::create("setup");
	entered = StaticCString::create("entered");
	exited = StaticCString::create("exited");
	updated = StaticCString::create("updated");
	_update = StaticCString::create("_update");
	state_changed = StaticCString::create("state_changed");
	_get_configuration_warning = StaticCString::create("_get_configuration_warning");
}