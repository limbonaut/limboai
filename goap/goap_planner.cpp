/**
 * goap_planner.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "goap_planner.h"

#ifdef LIMBOAI_MODULE
#include "core/os/os.h"
#include "core/string/print_string.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

// Debug flag - set to true to enable planner tracing
#define GOAP_PLANNER_DEBUG true

#ifdef LIMBOAI_MODULE
#define GOAP_DEBUG_PRINT(msg) if (GOAP_PLANNER_DEBUG) print_line(msg)
#else
#define GOAP_DEBUG_PRINT(msg) if (GOAP_PLANNER_DEBUG) UtilityFunctions::print(msg)
#endif

TypedArray<GOAPAction> GOAPPlanner::_reconstruct_plan(const Vector<PlannerNode> &p_closed_list, int p_goal_index) const {
	TypedArray<GOAPAction> plan;

	// Trace back from goal to start, collecting actions
	int current_index = p_goal_index;
	while (current_index != -1 && p_closed_list[current_index].action.is_valid()) {
		plan.push_back(p_closed_list[current_index].action);
		current_index = p_closed_list[current_index].parent_index;
	}

	// In backward search, trace-back from found node (current state) to goal state
	// gives us actions in correct execution order already (first action first)
	return plan;
}

TypedArray<GOAPAction> GOAPPlanner::plan(
		const TypedArray<GOAPAction> &p_available_actions,
		const Ref<GOAPWorldState> &p_current_state,
		const Ref<GOAPGoal> &p_goal,
		Node *p_agent,
		const Ref<Blackboard> &p_blackboard) {
	ERR_FAIL_COND_V(p_goal.is_null(), TypedArray<GOAPAction>());

	Ref<GOAPWorldState> goal_state = p_goal->create_world_state();
	return plan_to_state(p_available_actions, p_current_state, goal_state, p_agent, p_blackboard);
}

TypedArray<GOAPAction> GOAPPlanner::plan_to_state(
		const TypedArray<GOAPAction> &p_available_actions,
		const Ref<GOAPWorldState> &p_current_state,
		const Ref<GOAPWorldState> &p_goal_state,
		Node *p_agent,
		const Ref<Blackboard> &p_blackboard) {
	// Reset statistics
	last_iterations = 0;
	last_plan_time_ms = 0.0;

#ifdef LIMBOAI_MODULE
	uint64_t start_time = OS::get_singleton()->get_ticks_usec();
#else
	uint64_t start_time = Time::get_singleton()->get_ticks_usec();
#endif

	ERR_FAIL_COND_V(p_current_state.is_null(), TypedArray<GOAPAction>());
	ERR_FAIL_COND_V(p_goal_state.is_null(), TypedArray<GOAPAction>());

	// Debug: Print goal state
	GOAP_DEBUG_PRINT("=== GOAP PLANNER START ===");
	{
		TypedArray<StringName> goal_facts = p_goal_state->get_fact_names();
		String goal_str = "Goal state facts: ";
		for (int i = 0; i < goal_facts.size(); i++) {
			StringName fact = goal_facts[i];
			goal_str += String(fact) + "=" + String(p_goal_state->get_fact(fact)) + " ";
		}
		GOAP_DEBUG_PRINT(goal_str);

		TypedArray<StringName> current_facts = p_current_state->get_fact_names();
		String current_str = "Current state facts: ";
		for (int i = 0; i < current_facts.size(); i++) {
			StringName fact = current_facts[i];
			current_str += String(fact) + "=" + String(p_current_state->get_fact(fact)) + " ";
		}
		GOAP_DEBUG_PRINT(current_str);
	}

	// Check if goal is already satisfied
	if (p_current_state->satisfies(p_goal_state)) {
		GOAP_DEBUG_PRINT("Goal already satisfied - returning empty plan");
		return TypedArray<GOAPAction>(); // Empty plan - goal already achieved
	}

	// No actions available
	if (p_available_actions.is_empty()) {
		GOAP_DEBUG_PRINT("No actions available!");
		return TypedArray<GOAPAction>();
	}

	// Debug: List available actions
	GOAP_DEBUG_PRINT("Available actions (" + itos(p_available_actions.size()) + "):");
	for (int i = 0; i < p_available_actions.size(); i++) {
		Ref<GOAPAction> action = p_available_actions[i];
		if (action.is_valid()) {
			Dictionary effects = action->get_effects();
			Dictionary preconditions = action->get_preconditions();
			String action_str = "  " + String(action->get_action_name());
			action_str += " precond={";
			Array pkeys = preconditions.keys();
			for (int j = 0; j < pkeys.size(); j++) {
				if (j > 0) action_str += ", ";
				action_str += String(pkeys[j]) + ":" + String(preconditions[pkeys[j]]);
			}
			action_str += "} effects={";
			Array ekeys = effects.keys();
			for (int j = 0; j < ekeys.size(); j++) {
				if (j > 0) action_str += ", ";
				action_str += String(ekeys[j]) + ":" + String(effects[ekeys[j]]);
			}
			action_str += "}";
			GOAP_DEBUG_PRINT(action_str);
		}
	}

	// A* search using backward chaining
	// We search backwards from goal to current state
	// Start node is the goal state, we try to reach current state

	Vector<PlannerNode> open_list;
	Vector<PlannerNode> closed_list;

	// Hash-based lookup for O(1) state existence checks
	// Maps state hash -> index in respective list
	HashMap<uint32_t, Vector<int>> closed_hash; // Hash -> indices (for collision handling)
	HashMap<uint32_t, Vector<int>> open_hash;   // Hash -> indices (for collision handling)

	// Create start node (goal state)
	PlannerNode start_node;
	start_node.state = p_goal_state->duplicate();
	start_node.action = Ref<GOAPAction>();
	start_node.g_cost = 0;
	start_node.h_cost = p_goal_state->distance_to(p_current_state);
	start_node.parent_index = -1;

	open_list.push_back(start_node);
	uint32_t start_hash = start_node.state->compute_hash();
	open_hash[start_hash].push_back(0);

	GOAP_DEBUG_PRINT("Starting A* backward search...");

	while (!open_list.is_empty() && last_iterations < max_iterations) {
		last_iterations++;

		// Find node with lowest f_cost in open list
		int best_index = 0;
		int best_f_cost = open_list[0].f_cost();
		for (int i = 1; i < open_list.size(); i++) {
			int f_cost = open_list[i].f_cost();
			if (f_cost < best_f_cost) {
				best_f_cost = f_cost;
				best_index = i;
			}
		}

		// Move best node from open to closed list
		PlannerNode current = open_list[best_index];
		uint32_t current_hash = current.state->compute_hash();

		// Remove from open list and hash
		open_list.remove_at(best_index);
		// Update open_hash - remove old index and shift others
		if (open_hash.has(current_hash)) {
			Vector<int> &indices = open_hash[current_hash];
			indices.erase(best_index);
			if (indices.is_empty()) {
				open_hash.erase(current_hash);
			}
		}
		// Shift indices in open_hash for elements after best_index
		for (KeyValue<uint32_t, Vector<int>> &kv : open_hash) {
			for (int i = 0; i < kv.value.size(); i++) {
				if (kv.value[i] > best_index) {
					kv.value.write[i]--;
				}
			}
		}

		int current_closed_index = closed_list.size();
		closed_list.push_back(current);
		closed_hash[current_hash].push_back(current_closed_index);

		// Check if we've reached the current state (goal of backward search)
		GOAP_DEBUG_PRINT("Iteration " + itos(last_iterations) + ": checking if current state satisfies node...");
		{
			// Show what facts this node requires
			TypedArray<StringName> node_facts = current.state->get_fact_names();
			String node_str = "  Node requires: ";
			for (int i = 0; i < node_facts.size(); i++) {
				StringName fact = node_facts[i];
				node_str += String(fact) + "=" + String(current.state->get_fact(fact)) + " ";
			}
			GOAP_DEBUG_PRINT(node_str);

			// Check each fact
			bool all_satisfied = true;
			for (int i = 0; i < node_facts.size(); i++) {
				StringName fact = node_facts[i];
				Variant required = current.state->get_fact(fact);
				Variant actual = p_current_state->get_fact(fact);
				bool has_it = p_current_state->has_fact(fact);
				String check = "  Checking " + String(fact) + ": required=" + String(required) + " actual=" + String(actual) + " has=" + (has_it ? "true" : "false");
				if (has_it && actual == required) {
					check += " MATCH";
				} else {
					check += " FAIL";
					all_satisfied = false;
				}
				GOAP_DEBUG_PRINT(check);
			}
			GOAP_DEBUG_PRINT("  All satisfied: " + String(all_satisfied ? "YES" : "NO"));
		}

		if (p_current_state->satisfies(current.state)) {
			GOAP_DEBUG_PRINT("SUCCESS! Current state satisfies node - plan found!");
			// Found a plan! Reconstruct and return it
#ifdef LIMBOAI_MODULE
			uint64_t end_time = OS::get_singleton()->get_ticks_usec();
#else
			uint64_t end_time = Time::get_singleton()->get_ticks_usec();
#endif
			last_plan_time_ms = (end_time - start_time) / 1000.0;
			return _reconstruct_plan(closed_list, current_closed_index);
		}

		// Expand node: try each action that could lead to current state
		// In backward search, we look for actions whose EFFECTS satisfy some of our required preconditions
		GOAP_DEBUG_PRINT("Expanding node - checking which actions are useful...");
		for (int i = 0; i < p_available_actions.size(); i++) {
			Ref<GOAPAction> action = p_available_actions[i];
			if (action.is_null()) {
				continue;
			}

			// Check if this action's effects contribute to the current node's state
			// (i.e., does this action produce any fact that we need?)
			Dictionary effects = action->get_effects();
			Array effect_keys = effects.keys();
			bool action_useful = false;

			String action_name = String(action->get_action_name());
			for (int j = 0; j < effect_keys.size(); j++) {
				Variant key = effect_keys[j];
				StringName effect_fact = key;
				bool has_fact = current.state->has_fact(effect_fact);
				if (has_fact) {
					Variant effect_value = effects[key]; // Use original key for Dictionary lookup
					Variant required_value = current.state->get_fact(effect_fact);
					GOAP_DEBUG_PRINT("  Action " + action_name + ": effect " + String(effect_fact) + "=" + String(effect_value) + " vs required=" + String(required_value) + " match=" + String(effect_value == required_value ? "YES" : "NO"));
					if (effect_value == required_value) {
						action_useful = true;
						break;
					}
				}
			}

			if (!action_useful) {
				continue;
			}
			GOAP_DEBUG_PRINT("  -> Action " + action_name + " IS USEFUL!");

			// Create the predecessor state (state before this action was applied)
			// Remove the effects and add the preconditions
			Ref<GOAPWorldState> predecessor_state = current.state->duplicate();

			// Remove effects from predecessor (they won't exist yet)
			for (int j = 0; j < effect_keys.size(); j++) {
				StringName effect_fact = effect_keys[j];
				predecessor_state->erase_fact(effect_fact);
			}

			// Add preconditions to predecessor (they must be true for action to execute)
			Dictionary preconditions = action->get_preconditions();
			Array precondition_keys = preconditions.keys();
			for (int j = 0; j < precondition_keys.size(); j++) {
				Variant key = precondition_keys[j];
				StringName precondition_fact = key;
				predecessor_state->set_fact(precondition_fact, preconditions[key]); // Use original key for Dictionary lookup
			}

			// Check if this state is already in closed list using hash lookup (O(1) average)
			uint32_t pred_hash = predecessor_state->compute_hash();
			bool in_closed = false;
			if (closed_hash.has(pred_hash)) {
				// Hash collision possible - check actual equality
				const Vector<int> &indices = closed_hash[pred_hash];
				for (int idx : indices) {
					if (predecessor_state->equals(closed_list[idx].state)) {
						in_closed = true;
						break;
					}
				}
			}
			if (in_closed) {
				continue;
			}

			// Calculate costs
			int action_cost = action->get_cost(p_agent, p_blackboard);
			int new_g_cost = current.g_cost + action_cost;
			int new_h_cost = predecessor_state->distance_to(p_current_state);

			// Check if this state is already in open list using hash lookup (O(1) average)
			bool in_open = false;
			int existing_open_index = -1;
			if (open_hash.has(pred_hash)) {
				const Vector<int> &indices = open_hash[pred_hash];
				for (int idx : indices) {
					if (predecessor_state->equals(open_list[idx].state)) {
						in_open = true;
						existing_open_index = idx;
						break;
					}
				}
			}

			if (in_open && existing_open_index >= 0) {
				if (new_g_cost < open_list[existing_open_index].g_cost) {
					// Found better path, update
					open_list.write[existing_open_index].g_cost = new_g_cost;
					open_list.write[existing_open_index].h_cost = new_h_cost;
					open_list.write[existing_open_index].action = action;
					open_list.write[existing_open_index].parent_index = current_closed_index;
				}
			} else if (!in_open) {
				// Add new node to open list
				PlannerNode new_node;
				new_node.state = predecessor_state;
				new_node.action = action;
				new_node.g_cost = new_g_cost;
				new_node.h_cost = new_h_cost;
				new_node.parent_index = current_closed_index;
				int new_index = open_list.size();
				open_list.push_back(new_node);
				open_hash[pred_hash].push_back(new_index);
			}
		}
	}

	// No plan found
	GOAP_DEBUG_PRINT("=== NO PLAN FOUND ===");
	GOAP_DEBUG_PRINT("Iterations: " + itos(last_iterations) + "/" + itos(max_iterations));
	GOAP_DEBUG_PRINT("Open list empty: " + String(open_list.is_empty() ? "YES" : "NO"));
	GOAP_DEBUG_PRINT("Closed list size: " + itos(closed_list.size()));

#ifdef LIMBOAI_MODULE
	uint64_t end_time = OS::get_singleton()->get_ticks_usec();
#else
	uint64_t end_time = Time::get_singleton()->get_ticks_usec();
#endif
	last_plan_time_ms = (end_time - start_time) / 1000.0;

	return TypedArray<GOAPAction>();
}

void GOAPPlanner::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_max_iterations", "max"), &GOAPPlanner::set_max_iterations);
	ClassDB::bind_method(D_METHOD("get_max_iterations"), &GOAPPlanner::get_max_iterations);

	ClassDB::bind_method(D_METHOD("get_last_iterations"), &GOAPPlanner::get_last_iterations);
	ClassDB::bind_method(D_METHOD("get_last_plan_time_ms"), &GOAPPlanner::get_last_plan_time_ms);

	ClassDB::bind_method(D_METHOD("plan", "available_actions", "current_state", "goal", "agent", "blackboard"),
			&GOAPPlanner::plan, DEFVAL(Variant()), DEFVAL(Ref<Blackboard>()));
	ClassDB::bind_method(D_METHOD("plan_to_state", "available_actions", "current_state", "goal_state", "agent", "blackboard"),
			&GOAPPlanner::plan_to_state, DEFVAL(Variant()), DEFVAL(Ref<Blackboard>()));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_iterations", PROPERTY_HINT_RANGE, "10,10000,10"), "set_max_iterations", "get_max_iterations");
}
