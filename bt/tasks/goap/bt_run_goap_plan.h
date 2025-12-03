/**
 * bt_run_goap_plan.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_RUN_GOAP_PLAN_H
#define BT_RUN_GOAP_PLAN_H

#include "../bt_action.h"
#include "../../../goap/goap_action.h"
#include "../../../goap/goap_goal.h"
#include "../../../goap/goap_planner.h"
#include "../../behavior_tree.h"
#include "../../bt_instance.h"

#ifdef LIMBOAI_MODULE
#include "core/variant/typed_array.h"
#include "core/templates/hash_set.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/templates/hash_set.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

class BTRunGOAPPlan : public BTAction {
	GDCLASS(BTRunGOAPPlan, BTAction);
	TASK_CATEGORY(GOAP);

private:
	// Configuration
	Ref<GOAPGoal> goal;
	TypedArray<GOAPAction> available_actions;
	Ref<BehaviorTree> fallback_tree;
	float replan_cooldown = 0.2f;        // Minimum time between replans (throttle)
	float replan_debounce = 0.1f;        // Wait for world state to stabilize before replanning
	int max_iterations = 1000;

	// Runtime state
	Ref<GOAPPlanner> planner;
	TypedArray<GOAPAction> current_plan;
	int current_action_index = 0;
	Ref<BTInstance> current_action_instance;
	bool interrupt_requested = false;
	double time_since_last_replan = 0.0;
	double time_since_facts_changed = 0.0; // Debounce timer: time since world state last changed
	bool replan_pending = false;           // True if waiting for debounce before replanning
	bool plan_active = false;

	// Relevant facts tracking for smart replanning
	HashSet<StringName> relevant_facts;
	Dictionary cached_fact_values;

	// Fallback handling
	Ref<BTInstance> fallback_instance;
	bool executing_fallback = false;

	// Recursion guard
	static thread_local int execution_depth;
	static const int MAX_DEPTH = 3;

	// Internal methods
	void _cache_relevant_facts();
	bool _should_replan();
	void _invalidate_plan();
	TypedArray<StringName> _collect_all_relevant_facts() const;
	Ref<GOAPWorldState> _create_world_state() const;
	Status _execute_fallback(double p_delta);

protected:
	static void _bind_methods();

	virtual String _generate_name() override;
	virtual void _setup() override;
	virtual void _enter() override;
	virtual void _exit() override;
	virtual Status _tick(double p_delta) override;

public:
	void set_goal(const Ref<GOAPGoal> &p_goal) {
		goal = p_goal;
		emit_changed();
	}
	Ref<GOAPGoal> get_goal() const { return goal; }

	void set_available_actions(const TypedArray<GOAPAction> &p_actions) {
		available_actions = p_actions;
		emit_changed();
	}
	TypedArray<GOAPAction> get_available_actions() const { return available_actions; }

	void set_fallback_tree(const Ref<BehaviorTree> &p_tree) {
		fallback_tree = p_tree;
		emit_changed();
	}
	Ref<BehaviorTree> get_fallback_tree() const { return fallback_tree; }

	void set_replan_cooldown(float p_cooldown) {
		replan_cooldown = p_cooldown;
		emit_changed();
	}
	float get_replan_cooldown() const { return replan_cooldown; }

	void set_replan_debounce(float p_debounce) {
		replan_debounce = p_debounce;
		emit_changed();
	}
	float get_replan_debounce() const { return replan_debounce; }

	void set_max_iterations(int p_max) {
		max_iterations = p_max;
		emit_changed();
	}
	int get_max_iterations() const { return max_iterations; }

	// Force a replan on next tick
	void interrupt();

	// Get current plan for debugging
	TypedArray<GOAPAction> get_current_plan() const { return current_plan; }
	int get_current_action_index() const { return current_action_index; }
	bool is_plan_active() const { return plan_active; }

	BTRunGOAPPlan();
};

#endif // BT_RUN_GOAP_PLAN_H
