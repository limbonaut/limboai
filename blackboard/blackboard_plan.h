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
	// A derived plan can only have variables that exist in the base plan.
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

	void set_value(const String &p_name, const Variant &p_value);
	Variant get_value(const String &p_name) const;
	void add_var(const String &p_name, const BBVariable &p_var);
	void remove_var(const String &p_name);
	BBVariable get_var(const String &p_name);
	PackedStringArray list_vars() const;
	bool is_empty() const { return data.is_empty(); }

	void sync_with_base_plan();
	bool is_derived() { return base.is_valid(); }

	Ref<Blackboard> create_blackboard();
	void populate_blackboard(const Ref<Blackboard> &p_blackboard, bool overwrite);

	BlackboardPlan();
};

#endif // BLACKBOARD_PLAN_H
