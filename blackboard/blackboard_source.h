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
	HashMap<String, BBVariable> data;

	// When base is not null, the source is considered to be derived from the base source.
	// A derived source can only have variables that exist in the base source.
	Ref<BlackboardSource> base;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _property_can_revert(const StringName &p_name) const;
	bool _property_get_revert(const StringName &p_name, Variant &r_property) const;

public:
	void set_base_source(const Ref<BlackboardSource> &p_base);
	Ref<BlackboardSource> get_base_source() const { return base; }

	void set_value(const String &p_name, const Variant &p_value);
	Variant get_value(const String &p_name) const;
	void add_var(const String &p_name, const BBVariable &p_var);
	void remove_var(const String &p_name);
	BBVariable get_var(const String &p_name);
	PackedStringArray list_vars() const;
	bool is_empty() const { return data.is_empty(); }

	void sync_with_base_source();
	bool is_derived() { return base.is_valid(); }

	Ref<Blackboard> create_blackboard();
	void populate_blackboard(const Ref<Blackboard> &p_blackboard, bool overwrite);

	BlackboardSource();
};

#endif // BLACKBOARD_SOURCE_H
