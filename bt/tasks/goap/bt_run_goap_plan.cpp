/**
 * bt_run_goap_plan.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_run_goap_plan.h"

#ifdef LIMBOAI_MODULE
#include "core/error/error_macros.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/core/error_macros.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

thread_local int BTRunGOAPPlan::execution_depth = 0;

BTRunGOAPPlan::BTRunGOAPPlan() {
	planner.instantiate();
}

String BTRunGOAPPlan::_generate_name() {
	if (goal.is_valid() && !goal->get_goal_name().is_empty()) {
		return vformat("RunGOAPPlan: %s", goal->get_goal_name());
	}
	return "RunGOAPPlan";
}

void BTRunGOAPPlan::_setup() {
	planner->set_max_iterations(max_iterations);
}

void BTRunGOAPPlan::_enter() {
	current_plan.clear();
	current_action_index = 0;
	current_action_instance.unref();
	interrupt_requested = false;
	time_since_last_replan = replan_cooldown; // Allow immediate planning
	time_since_facts_changed = replan_debounce; // Allow immediate planning
	replan_pending = false;
	plan_active = false;
	executing_fallback = false;
	fallback_instance.unref();
	relevant_facts.clear();
	cached_fact_values.clear();
}

void BTRunGOAPPlan::_exit() {
	// Clean up any running action
	if (current_action_instance.is_valid()) {
		current_action_instance.unref();
	}
	if (fallback_instance.is_valid()) {
		fallback_instance.unref();
	}
	plan_active = false;
	executing_fallback = false;
}

TypedArray<StringName> BTRunGOAPPlan::_collect_all_relevant_facts() const {
	TypedArray<StringName> facts;

	// Collect facts from goal
	if (goal.is_valid()) {
		TypedArray<StringName> goal_facts = goal->get_required_facts();
		for (int i = 0; i < goal_facts.size(); i++) {
			if (!facts.has(goal_facts[i])) {
				facts.append(goal_facts[i]);
			}
		}
	}

	// Collect facts from all actions
	for (int i = 0; i < available_actions.size(); i++) {
		Ref<GOAPAction> action = available_actions[i];
		if (action.is_valid()) {
			TypedArray<StringName> action_facts = action->get_relevant_facts();
			for (int j = 0; j < action_facts.size(); j++) {
				if (!facts.has(action_facts[j])) {
					facts.append(action_facts[j]);
				}
			}
		}
	}

	return facts;
}

Ref<GOAPWorldState> BTRunGOAPPlan::_create_world_state() const {
	Ref<GOAPWorldState> world_state;
	world_state.instantiate();

	Ref<Blackboard> bb = get_blackboard();
	if (bb.is_valid()) {
		TypedArray<StringName> facts = _collect_all_relevant_facts();
		world_state->populate_from_blackboard(bb, facts);
	}

	return world_state;
}

void BTRunGOAPPlan::_cache_relevant_facts() {
	relevant_facts.clear();
	cached_fact_values.clear();

	Ref<Blackboard> bb = get_blackboard();
	if (bb.is_null()) {
		return;
	}

	// Cache facts from current plan's preconditions
	for (int i = current_action_index; i < current_plan.size(); i++) {
		Ref<GOAPAction> action = current_plan[i];
		if (action.is_null()) {
			continue;
		}

		Dictionary preconditions = action->get_preconditions();
		Array keys = preconditions.keys();
		for (int j = 0; j < keys.size(); j++) {
			StringName fact = keys[j];
			if (!relevant_facts.has(fact)) {
				relevant_facts.insert(fact);
				cached_fact_values[fact] = bb->get_var(fact, Variant(), false);
			}
		}
	}
}

bool BTRunGOAPPlan::_should_replan() {
	if (interrupt_requested) {
		return true;
	}

	// Check if any relevant fact has changed
	Ref<Blackboard> bb = get_blackboard();
	if (bb.is_null()) {
		return false;
	}

	for (const StringName &fact : relevant_facts) {
		Variant current_value = bb->get_var(fact, Variant(), false);
		if (cached_fact_values.has(fact)) {
			if (current_value != cached_fact_values[fact]) {
				return true;
			}
		}
	}

	return false;
}

void BTRunGOAPPlan::_invalidate_plan() {
	current_plan.clear();
	current_action_index = 0;
	current_action_instance.unref();
	plan_active = false;
	relevant_facts.clear();
	cached_fact_values.clear();
}

void BTRunGOAPPlan::interrupt() {
	interrupt_requested = true;
}

BTRunGOAPPlan::Status BTRunGOAPPlan::_execute_fallback(double p_delta) {
	if (fallback_tree.is_null()) {
		return FAILURE;
	}

	// Initialize fallback instance if needed
	if (fallback_instance.is_null()) {
		fallback_instance = fallback_tree->instantiate(get_agent(), get_blackboard(), get_agent(), get_scene_root());
		if (fallback_instance.is_null()) {
			ERR_PRINT("BTRunGOAPPlan: Failed to instantiate fallback tree.");
			return FAILURE;
		}
		executing_fallback = true;
	}

	// Execute fallback
	Status status = fallback_instance->update(p_delta);

	if (status != RUNNING) {
		// Fallback completed, clean up
		fallback_instance.unref();
		executing_fallback = false;
	}

	return status;
}

BTRunGOAPPlan::Status BTRunGOAPPlan::_tick(double p_delta) {
	// Recursion guard
	if (execution_depth >= MAX_DEPTH) {
		ERR_PRINT("BTRunGOAPPlan: Maximum nesting depth exceeded. Possible recursion in fallback tree.");
		return FAILURE;
	}
	execution_depth++;

	// Update timers
	time_since_last_replan += p_delta;
	time_since_facts_changed += p_delta;

	// If executing fallback, continue with it
	if (executing_fallback) {
		Status fallback_status = _execute_fallback(p_delta);
		execution_depth--;

		if (fallback_status != RUNNING) {
			// Try to replan after fallback completes
			_invalidate_plan();
		}
		return fallback_status;
	}

	// Check if facts have changed (triggers debounce)
	bool facts_changed = _should_replan();
	if (facts_changed) {
		// Reset debounce timer - wait for world state to stabilize
		time_since_facts_changed = 0.0;
		replan_pending = true;
	}

	// Determine if we should actually replan now
	// Replan if:
	// 1. No active plan, OR
	// 2. Interrupt requested (immediate), OR
	// 3. Replan pending AND debounce timer expired AND cooldown satisfied
	bool need_immediate_replan = !plan_active || current_plan.is_empty() || interrupt_requested;
	bool debounce_ready = replan_pending && time_since_facts_changed >= replan_debounce;
	bool cooldown_ready = time_since_last_replan >= replan_cooldown;

	if ((need_immediate_replan && cooldown_ready) || (debounce_ready && cooldown_ready)) {
		// Clean up current action if running
		if (current_action_instance.is_valid()) {
			current_action_instance.unref();
		}

		// Create world state from blackboard
		Ref<GOAPWorldState> world_state = _create_world_state();

		// Plan
		current_plan = planner->plan(available_actions, world_state, goal, get_agent(), get_blackboard());
		current_action_index = 0;
		interrupt_requested = false;
		time_since_last_replan = 0.0;
		replan_pending = false; // Clear debounce pending state

		if (current_plan.is_empty()) {
			// No plan found - check if goal is already satisfied
			if (goal.is_valid() && goal->is_satisfied(world_state)) {
				plan_active = false;
				execution_depth--;
				return SUCCESS;
			}

			// Execute fallback
			plan_active = false;
			Status fallback_status = _execute_fallback(p_delta);
			execution_depth--;
			return fallback_status;
		}

		plan_active = true;
		_cache_relevant_facts();
	}

	// Execute current action in plan
	if (current_plan.is_empty() || current_action_index >= current_plan.size()) {
		plan_active = false;
		execution_depth--;
		return SUCCESS; // Plan complete
	}

	Ref<GOAPAction> current_action = current_plan[current_action_index];
	if (current_action.is_null()) {
		ERR_PRINT("BTRunGOAPPlan: Null action in plan.");
		_invalidate_plan();
		execution_depth--;
		return FAILURE;
	}

	// Initialize action instance if needed
	if (current_action_instance.is_null()) {
		// Check procedural preconditions before starting action
		if (!current_action->check_procedural_preconditions(get_agent(), get_blackboard())) {
			// Procedural precondition failed - replan
			_invalidate_plan();
			execution_depth--;
			return RUNNING; // Will replan on next tick
		}

		// Get the execution tree for this action
		Ref<BehaviorTree> action_tree = current_action->get_execution_tree();
		if (action_tree.is_null()) {
			ERR_PRINT(vformat("BTRunGOAPPlan: Action '%s' has no execution tree.", current_action->get_action_name()));
			_invalidate_plan();
			execution_depth--;
			return FAILURE;
		}

		current_action_instance = action_tree->instantiate(get_agent(), get_blackboard(), get_agent(), get_scene_root());
		if (current_action_instance.is_null()) {
			ERR_PRINT(vformat("BTRunGOAPPlan: Failed to instantiate execution tree for action '%s'.", current_action->get_action_name()));
			_invalidate_plan();
			execution_depth--;
			return FAILURE;
		}
	}

	// Tick current action
	Status action_status = current_action_instance->update(p_delta);

	if (action_status == SUCCESS) {
		// Action completed successfully - move to next action
		current_action_instance.unref();
		current_action_index++;
		_cache_relevant_facts(); // Update cached facts for remaining actions

		if (current_action_index >= current_plan.size()) {
			// Plan complete!
			plan_active = false;
			execution_depth--;
			return SUCCESS;
		}
	} else if (action_status == FAILURE) {
		// Action failed - invalidate plan and replan
		_invalidate_plan();
		// Will replan on next tick (if cooldown allows)
	}

	execution_depth--;
	return RUNNING;
}

void BTRunGOAPPlan::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_goal", "goal"), &BTRunGOAPPlan::set_goal);
	ClassDB::bind_method(D_METHOD("get_goal"), &BTRunGOAPPlan::get_goal);

	ClassDB::bind_method(D_METHOD("set_available_actions", "actions"), &BTRunGOAPPlan::set_available_actions);
	ClassDB::bind_method(D_METHOD("get_available_actions"), &BTRunGOAPPlan::get_available_actions);

	ClassDB::bind_method(D_METHOD("set_fallback_tree", "tree"), &BTRunGOAPPlan::set_fallback_tree);
	ClassDB::bind_method(D_METHOD("get_fallback_tree"), &BTRunGOAPPlan::get_fallback_tree);

	ClassDB::bind_method(D_METHOD("set_replan_cooldown", "cooldown"), &BTRunGOAPPlan::set_replan_cooldown);
	ClassDB::bind_method(D_METHOD("get_replan_cooldown"), &BTRunGOAPPlan::get_replan_cooldown);

	ClassDB::bind_method(D_METHOD("set_replan_debounce", "debounce"), &BTRunGOAPPlan::set_replan_debounce);
	ClassDB::bind_method(D_METHOD("get_replan_debounce"), &BTRunGOAPPlan::get_replan_debounce);

	ClassDB::bind_method(D_METHOD("set_max_iterations", "max"), &BTRunGOAPPlan::set_max_iterations);
	ClassDB::bind_method(D_METHOD("get_max_iterations"), &BTRunGOAPPlan::get_max_iterations);

	ClassDB::bind_method(D_METHOD("interrupt"), &BTRunGOAPPlan::interrupt);
	ClassDB::bind_method(D_METHOD("get_current_plan"), &BTRunGOAPPlan::get_current_plan);
	ClassDB::bind_method(D_METHOD("get_current_action_index"), &BTRunGOAPPlan::get_current_action_index);
	ClassDB::bind_method(D_METHOD("is_plan_active"), &BTRunGOAPPlan::is_plan_active);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "goal", PROPERTY_HINT_RESOURCE_TYPE, "GOAPGoal"), "set_goal", "get_goal");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "available_actions", PROPERTY_HINT_ARRAY_TYPE, "GOAPAction"), "set_available_actions", "get_available_actions");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fallback_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviorTree"), "set_fallback_tree", "get_fallback_tree");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "replan_cooldown", PROPERTY_HINT_RANGE, "0.0,5.0,0.05"), "set_replan_cooldown", "get_replan_cooldown");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "replan_debounce", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_replan_debounce", "get_replan_debounce");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_iterations", PROPERTY_HINT_RANGE, "10,10000,10"), "set_max_iterations", "get_max_iterations");
}
