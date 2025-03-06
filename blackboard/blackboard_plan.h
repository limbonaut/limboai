/**
 * blackboard_plan.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BLACKBOARD_PLAN_H
#define BLACKBOARD_PLAN_H

#include "bb_variable.h"
#include "blackboard.h"
#include "callable_fallback_chain.h"

#ifdef LIMBOAI_MODULE
#include "core/io/resource.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

class BlackboardPlan : public Resource {
	GDCLASS(BlackboardPlan, Resource);

private:
	List<Pair<StringName, BBVariable>> var_list;
	HashMap<StringName, BBVariable> var_map;

	// When base is not null, the plan is considered to be derived from the base plan.
	// A derived plan can only have variables that exist in the base plan,
	// and only the values can be different in those variables.
	// The derived plan is synced with the base plan to maintain consistency.
	Ref<BlackboardPlan> base;

	// Mapping between variables in this plan and their parent scope names.
	// Used for linking variables to their parent scope counterparts upon Blackboard creation/population.
	HashMap<StringName, StringName> parent_scope_mapping;
	// A chain of callables that return parent scope plans (Ref<BlackboardPlan>).
	// If a callable is not valid, the next one in the chain is tried.
	// When at least one callable in a chain is valid, it enables variable mapping in the inspector.
	CallableFallbackChain parent_scope_plan_providers;

	// Bindings to properties in the scene to which this plan belongs.
	HashMap<StringName, NodePath> property_bindings;
	bool property_binding_enabled = false;

	// If true, NodePath variables will be prefetched, so that the vars will contain node pointers instead (upon BB creation/population).
	bool prefetch_nodepath_vars = true;

	_FORCE_INLINE_ bool _is_var_nil(const BBVariable &p_var) const { return p_var.get_type() == Variant::NIL; }
	_FORCE_INLINE_ bool _is_var_private(const String &p_name, const BBVariable &p_var) const { return is_derived() && p_name.begins_with("_"); }

protected:
	static void _bind_methods();

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _property_can_revert(const StringName &p_name) const;
	bool _property_get_revert(const StringName &p_name, Variant &r_property) const;

#ifdef LIMBOAI_GDEXTENSION
	String _to_string() const { return "<" + get_class() + "#" + itos(get_instance_id()) + ">"; }
#endif

public:
	void set_base_plan(const Ref<BlackboardPlan> &p_base);
	Ref<BlackboardPlan> get_base_plan() const { return base; }

	void set_parent_scope_plan_provider(const Callable &p_parent_scope_plan_provider);
	Callable get_parent_scope_plan_provider() const { return parent_scope_plan_providers.get_most_recent_valid(); }

	bool is_mapping_enabled() const;
	bool has_mapping(const StringName &p_name) const;

	bool has_property_binding(const StringName &p_name) const { return property_bindings.has(p_name); }
	void set_property_binding(const StringName &p_name, const NodePath &p_path);
	NodePath get_property_binding(const StringName &p_name) const { return property_bindings.has(p_name) ? property_bindings[p_name] : NodePath(); }

	void set_prefetch_nodepath_vars(bool p_enable);
	bool is_prefetching_nodepath_vars() const;

	void add_var(const StringName &p_name, const BBVariable &p_var);
	void remove_var(const StringName &p_name);
	BBVariable get_var(const StringName &p_name);
	Pair<StringName, BBVariable> get_var_by_index(int p_index);
	_FORCE_INLINE_ bool has_var(const StringName &p_name) { return var_map.has(p_name); }
	_FORCE_INLINE_ bool is_empty() const { return var_map.is_empty(); }
	int get_var_count() const { return var_map.size(); }

	TypedArray<StringName> list_vars() const;
	StringName get_var_name(const BBVariable &p_var) const;
	bool is_valid_var_name(const StringName &p_name) const;
	void rename_var(const StringName &p_name, const StringName &p_new_name);
	void move_var(int p_index, int p_new_index);

	void sync_with_base_plan();
	_FORCE_INLINE_ bool is_derived() const { return base.is_valid(); }
	_FORCE_INLINE_ bool is_derived_var_changed(const StringName &p_name) const { return base.is_valid() && var_map.has(p_name) && var_map[p_name].is_value_changed(); }

	Ref<Blackboard> create_blackboard(Node *p_prefetch_root, const Ref<Blackboard> &p_parent_scope = Ref<Blackboard>(), Node *p_prefetch_root_for_base_plan = nullptr);
	void populate_blackboard(const Ref<Blackboard> &p_blackboard, bool overwrite, Node *p_prefetch_root, Node *p_prefetch_root_for_base_plan = nullptr);

	BlackboardPlan();
};

#endif // BLACKBOARD_PLAN_H
