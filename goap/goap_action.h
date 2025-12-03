/**
 * goap_action.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef GOAP_ACTION_H
#define GOAP_ACTION_H

#include "goap_world_state.h"
#include "../blackboard/blackboard.h"

#ifdef LIMBOAI_MODULE
#include "core/io/resource.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/variant/dictionary.h"
#include "scene/main/node.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/dictionary.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

class BehaviorTree;

class GOAPAction : public Resource {
	GDCLASS(GOAPAction, Resource);

private:
	StringName action_name;
	Dictionary preconditions;
	Dictionary effects;
	int base_cost = 1;
	Ref<Resource> execution_tree; // BehaviorTree - forward declared to avoid circular dependency

protected:
	static void _bind_methods();

	// Virtual method for procedural precondition checks (called at execution time)
	GDVIRTUAL2RC(bool, _check_procedural_preconditions, Node *, Ref<Blackboard>);

	// Virtual method for dynamic cost calculation
	GDVIRTUAL3RC(int, _get_dynamic_cost, Node *, Ref<Blackboard>, int);

public:
	void set_action_name(const StringName &p_name) {
		action_name = p_name;
		emit_changed();
	}
	StringName get_action_name() const { return action_name; }

	void set_preconditions(const Dictionary &p_preconditions) {
		preconditions = p_preconditions;
		emit_changed();
	}
	Dictionary get_preconditions() const { return preconditions; }

	void set_effects(const Dictionary &p_effects) {
		effects = p_effects;
		emit_changed();
	}
	Dictionary get_effects() const { return effects; }

	void set_base_cost(int p_cost) {
		base_cost = p_cost;
		emit_changed();
	}
	int get_base_cost() const { return base_cost; }

	void set_execution_tree(const Ref<Resource> &p_tree) {
		execution_tree = p_tree;
		emit_changed();
	}
	Ref<Resource> get_execution_tree() const { return execution_tree; }

	// Check if action's preconditions are satisfied by the world state
	// Used during planning (fast, symbolic only)
	bool is_valid(const Ref<GOAPWorldState> &p_state) const;

	// Apply action's effects to a world state (returns new state)
	Ref<GOAPWorldState> apply_effects_to_state(const Ref<GOAPWorldState> &p_state) const;

	// Get cost for planning (may call virtual for dynamic cost)
	int get_cost(Node *p_agent = nullptr, const Ref<Blackboard> &p_blackboard = Ref<Blackboard>()) const;

	// Check procedural preconditions (called at execution time)
	// May perform expensive operations like raycasts
	bool check_procedural_preconditions(Node *p_agent, const Ref<Blackboard> &p_blackboard) const;

	// Get all fact names this action uses (preconditions + effects)
	TypedArray<StringName> get_relevant_facts() const;

	// Check if this action can contribute to achieving a specific fact
	bool produces_fact(const StringName &p_fact_name) const;

	GOAPAction() {}
};

#endif // GOAP_ACTION_H
