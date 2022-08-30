/* limbo_string_names.h */

#ifndef LIMBO_STRING_NAMES_H
#define LIMBO_STRING_NAMES_H

#include "core/string_name.h"
#include "core/typedefs.h"

class LimboStringNames {
	friend void register_limboai_types();
	friend void unregister_limboai_types();

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
};

#endif // LIMBO_STRING_NAMES_H