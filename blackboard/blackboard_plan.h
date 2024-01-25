/**
 * blackboard_plan.h
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BLACKBOARD_PLAN_H
#define BLACKBOARD_PLAN_H

#include "core/io/resource.h"

#include "bb_variable.h"
#include "blackboard.h"

class BlackboardPlan : public Resource {
	GDCLASS(BlackboardPlan, Resource);

private:
	HashMap<String, BBVariable> data;

	// When base is not null, the plan is considered to be derived from the base plan.
	// A derived plan can only have variables that exist in the base plan,
	// and only the values can be different in those variables.
	Ref<BlackboardPlan> base;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _property_can_revert(const StringName &p_name) const;
	bool _property_get_revert(const StringName &p_name, Variant &r_property) const;

public:
	void set_base_plan(const Ref<BlackboardPlan> &p_base);
	Ref<BlackboardPlan> get_base_plan() const { return base; }

	void add_var(const String &p_name, const BBVariable &p_var);
	void remove_var(const String &p_name);
	BBVariable get_var(const String &p_name);
	Pair<String, BBVariable> get_var_by_index(int p_index);
	bool has_var(const String &p_name) { return data.has(p_name); }
	bool is_empty() const { return data.is_empty(); }
	int get_var_count() const { return data.size(); }

	PackedStringArray list_vars() const;
	String get_var_name(const BBVariable &p_var) const;
	void rename_var(const String &p_name, const String &p_new_name);
	void swap_vars(int idx_a, int idx_b);

	void sync_with_base_plan();
	bool is_derived() const { return base.is_valid(); }

	Ref<Blackboard> create_blackboard();
	void populate_blackboard(const Ref<Blackboard> &p_blackboard, bool overwrite);

	BlackboardPlan();
};

#endif // BLACKBOARD_PLAN_H
