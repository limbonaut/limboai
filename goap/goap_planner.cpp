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

// Debug flag - only enabled in debug builds
#ifdef DEBUG_ENABLED
#define GOAP_PLANNER_DEBUG false // Set to true for verbose planner tracing
#else
#define GOAP_PLANNER_DEBUG false
#endif

#ifdef LIMBOAI_MODULE
#define GOAP_DEBUG_PRINT(msg) if (GOAP_PLANNER_DEBUG) print_line(msg)
#else
#define GOAP_DEBUG_PRINT(msg) if (GOAP_PLANNER_DEBUG) UtilityFunctions::print(msg)
#endif

// ============================================================================
// Binary Min-Heap Operations for O(log n) Priority Queue
// ============================================================================

void GOAPPlanner::_heap_push(Vector<HeapEntry> &heap, HeapEntry entry) {
	heap.push_back(entry);
	_heap_sift_up(heap, heap.size() - 1);
}

GOAPPlanner::HeapEntry GOAPPlanner::_heap_pop(Vector<HeapEntry> &heap) {
	HeapEntry invalid_entry = { -1, INT_MAX };
	ERR_FAIL_COND_V(heap.is_empty(), invalid_entry);

	HeapEntry result = heap[0];
	int last_idx = heap.size() - 1;

	if (last_idx > 0) {
		heap.write[0] = heap[last_idx];
		heap.resize(last_idx);
		_heap_sift_down(heap, 0);
	} else {
		heap.clear();
	}

	return result;
}

void GOAPPlanner::_heap_sift_up(Vector<HeapEntry> &heap, int index) {
	while (index > 0) {
		int parent = (index - 1) / 2;
		if (heap[parent].f_cost <= heap[index].f_cost) {
			break;
		}
		// Swap with parent
		HeapEntry temp = heap[parent];
		heap.write[parent] = heap[index];
		heap.write[index] = temp;
		index = parent;
	}
}

void GOAPPlanner::_heap_sift_down(Vector<HeapEntry> &heap, int index) {
	int size = heap.size();
	while (true) {
		int smallest = index;
		int left = 2 * index + 1;
		int right = 2 * index + 2;

		if (left < size && heap[left].f_cost < heap[smallest].f_cost) {
			smallest = left;
		}
		if (right < size && heap[right].f_cost < heap[smallest].f_cost) {
			smallest = right;
		}

		if (smallest == index) {
			break;
		}

		// Swap with smallest child
		HeapEntry temp = heap[index];
		heap.write[index] = heap[smallest];
		heap.write[smallest] = temp;
		index = smallest;
	}
}

// ============================================================================
// Plan Reconstruction
// ============================================================================

TypedArray<GOAPAction> GOAPPlanner::_reconstruct_plan(const Vector<PlannerNode> &p_node_pool, int p_goal_index) const {
	TypedArray<GOAPAction> plan;

	// Trace back from goal to start, collecting actions
	int current_index = p_goal_index;
	while (current_index != -1 && p_node_pool[current_index].action.is_valid()) {
		plan.push_back(p_node_pool[current_index].action);
		current_index = p_node_pool[current_index].parent_index;
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
	if (GOAP_PLANNER_DEBUG) {
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
	if (GOAP_PLANNER_DEBUG) {
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
	}

	// ========================================================================
	// A* search using backward chaining with priority queue (min-heap)
	// ========================================================================
	// We search backwards from goal to current state.
	// Start node is the goal state, we try to reach current state.
	//
	// Data structures:
	// - node_pool: All nodes created during search (stable indices)
	// - open_heap: Min-heap of (node_index, f_cost) for O(log n) extraction
	// - state_hash_to_node: Maps state hash -> node indices for O(1) lookup
	// - PlannerNode.in_closed: Lazy deletion flag (skip if already processed)

	Vector<PlannerNode> node_pool;
	Vector<HeapEntry> open_heap;
	HashMap<uint32_t, Vector<int>> state_hash_to_node; // Hash -> node indices

	// Create start node (goal state in backward search)
	PlannerNode start_node;
	start_node.state = p_goal_state->duplicate();
	start_node.action = Ref<GOAPAction>();
	start_node.g_cost = 0;
	start_node.h_cost = p_goal_state->distance_to(p_current_state);
	start_node.parent_index = -1;
	start_node.state_hash = start_node.state->compute_hash();
	start_node.in_closed = false;

	int start_index = node_pool.size();
	node_pool.push_back(start_node);
	state_hash_to_node[start_node.state_hash].push_back(start_index);
	_heap_push(open_heap, HeapEntry{ start_index, start_node.f_cost() });

	GOAP_DEBUG_PRINT("Starting A* backward search with priority queue...");

	while (!open_heap.is_empty() && last_iterations < max_iterations) {
		last_iterations++;

		// Pop lowest f_cost node from heap - O(log n)
		HeapEntry best_entry = _heap_pop(open_heap);
		int current_index = best_entry.node_index;

		// Lazy deletion: skip if already in closed set
		if (node_pool[current_index].in_closed) {
			continue;
		}

		// Mark as closed (processed)
		node_pool.write[current_index].in_closed = true;

		// IMPORTANT: Copy data we need from current node BEFORE the action loop.
		// We cannot keep a reference to node_pool elements because push_back() may
		// reallocate the vector and invalidate all references.
		Ref<GOAPWorldState> current_state = node_pool[current_index].state;
		int current_g_cost = node_pool[current_index].g_cost;

		// Debug: Show current node state
		if (GOAP_PLANNER_DEBUG) {
			GOAP_DEBUG_PRINT("Iteration " + itos(last_iterations) + ": checking node with f_cost=" + itos(node_pool[current_index].f_cost()));
			TypedArray<StringName> node_facts = current_state->get_fact_names();
			String node_str = "  Node requires: ";
			for (int i = 0; i < node_facts.size(); i++) {
				StringName fact = node_facts[i];
				node_str += String(fact) + "=" + String(current_state->get_fact(fact)) + " ";
			}
			GOAP_DEBUG_PRINT(node_str);
		}

		// Check if we've reached the current state (goal of backward search)
		if (p_current_state->satisfies(current_state)) {
			GOAP_DEBUG_PRINT("SUCCESS! Current state satisfies node - plan found!");
#ifdef LIMBOAI_MODULE
			uint64_t end_time = OS::get_singleton()->get_ticks_usec();
#else
			uint64_t end_time = Time::get_singleton()->get_ticks_usec();
#endif
			last_plan_time_ms = (end_time - start_time) / 1000.0;
			return _reconstruct_plan(node_pool, current_index);
		}

		// Expand node: try each action that could lead to current state
		// In backward search, we look for actions whose EFFECTS satisfy some of our needs
		for (int i = 0; i < p_available_actions.size(); i++) {
			Ref<GOAPAction> action = p_available_actions[i];
			if (action.is_null()) {
				continue;
			}

			// Check if this action's effects contribute to the current node's state
			Dictionary effects = action->get_effects();
			Array effect_keys = effects.keys();
			bool action_useful = false;

			for (int j = 0; j < effect_keys.size(); j++) {
				StringName effect_fact = effect_keys[j];
				if (current_state->has_fact(effect_fact)) {
					Variant effect_value = effects[effect_keys[j]];
					Variant required_value = current_state->get_fact(effect_fact);
					if (effect_value == required_value) {
						action_useful = true;
						break;
					}
				}
			}

			if (!action_useful) {
				continue;
			}

			// Create the predecessor state (state before this action was applied)
			Ref<GOAPWorldState> predecessor_state = current_state->duplicate();

			// Remove effects from predecessor (they won't exist yet)
			for (int j = 0; j < effect_keys.size(); j++) {
				predecessor_state->erase_fact(effect_keys[j]);
			}

			// Add preconditions to predecessor (they must be true for action to execute)
			Dictionary preconditions = action->get_preconditions();
			Array precondition_keys = preconditions.keys();
			for (int j = 0; j < precondition_keys.size(); j++) {
				StringName precondition_fact = precondition_keys[j];
				predecessor_state->set_fact(precondition_fact, preconditions[precondition_keys[j]]);
			}

			// Calculate costs
			int action_cost = action->get_cost(p_agent, p_blackboard);
			int new_g_cost = current_g_cost + action_cost;
			int new_h_cost = predecessor_state->distance_to(p_current_state);
			uint32_t pred_hash = predecessor_state->compute_hash();

			// Check if this state already exists in node pool
			int existing_node_index = -1;
			if (state_hash_to_node.has(pred_hash)) {
				const Vector<int> &indices = state_hash_to_node[pred_hash];
				for (int idx : indices) {
					if (predecessor_state->equals(node_pool[idx].state)) {
						existing_node_index = idx;
						break;
					}
				}
			}

			if (existing_node_index >= 0) {
				PlannerNode &existing = node_pool.write[existing_node_index];
				// Skip if already in closed set
				if (existing.in_closed) {
					continue;
				}
				// Update if we found a better path
				if (new_g_cost < existing.g_cost) {
					existing.g_cost = new_g_cost;
					existing.h_cost = new_h_cost;
					existing.action = action;
					existing.parent_index = current_index;
					// Re-add to heap with new priority (old entry will be skipped via lazy deletion)
					_heap_push(open_heap, HeapEntry{ existing_node_index, existing.f_cost() });
				}
			} else {
				// Create new node
				PlannerNode new_node;
				new_node.state = predecessor_state;
				new_node.action = action;
				new_node.g_cost = new_g_cost;
				new_node.h_cost = new_h_cost;
				new_node.parent_index = current_index;
				new_node.state_hash = pred_hash;
				new_node.in_closed = false;

				int new_index = node_pool.size();
				node_pool.push_back(new_node);
				state_hash_to_node[pred_hash].push_back(new_index);
				_heap_push(open_heap, HeapEntry{ new_index, new_node.f_cost() });
			}
		}
	}

	// No plan found
	GOAP_DEBUG_PRINT("=== NO PLAN FOUND ===");
	GOAP_DEBUG_PRINT("Iterations: " + itos(last_iterations) + "/" + itos(max_iterations));
	GOAP_DEBUG_PRINT("Open heap empty: " + String(open_heap.is_empty() ? "YES" : "NO"));
	GOAP_DEBUG_PRINT("Node pool size: " + itos(node_pool.size()));

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
