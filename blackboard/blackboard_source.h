/**
 * blackboard_source.h
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BLACKBOARD_SOURCE_H
#define BLACKBOARD_SOURCE_H

#include "core/io/resource.h"

#include "bb_variable.h"
#include "blackboard.h"

class BlackboardSource : public Resource {
	GDCLASS(BlackboardSource, Resource);

private:
	HashMap<String, BBVariable> vars;
	Ref<BlackboardSource> base;
	// HashMap<String, BBVariable> overrides;

public:
	void set_value(const String &p_name, const Variant &p_value);
	Variant get_value(const String &p_name) const;
	void add_var(const String &p_name, const BBVariable &p_var);
	void remove_var(const String &p_name);
	BBVariable get_var(const String &p_name);
	PackedStringArray list_vars() const;

	void sync_base();
	Ref<Blackboard> instantiate();

	BlackboardSource() = default;
};

#endif // BLACKBOARD_SOURCE_H
