/**
 * limbo_string_names.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBO_STRING_NAMES_H
#define LIMBO_STRING_NAMES_H

#include "modules/register_module_types.h"

#include "core/string/string_name.h"
#include "core/typedefs.h"

class LimboStringNames {
	friend void initialize_limboai_module(ModuleInitializationLevel p_level);
	friend void uninitialize_limboai_module(ModuleInitializationLevel p_level);

	static LimboStringNames *singleton;

	static void create() { singleton = memnew(LimboStringNames); }
	static void free() {
		memdelete(singleton);
		singleton = nullptr;
	}

	LimboStringNames();

public:
	_FORCE_INLINE_ static LimboStringNames *get_singleton() { return singleton; }

	StringName _generate_name;
	StringName _setup;
	StringName _enter;
	StringName _exit;
	StringName _tick;
	StringName behavior_tree_finished;
	StringName setup;
	StringName entered;
	StringName exited;
	StringName updated;
	StringName _update;
	StringName state_changed;
	StringName _get_configuration_warning;
};

#endif // LIMBO_STRING_NAMES_H