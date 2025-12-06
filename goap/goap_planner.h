/**
 * goap_planner.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef GOAP_PLANNER_H
#define GOAP_PLANNER_H

#include "goap_action.h"
#include "goap_goal.h"
#include "goap_world_state.h"

#ifdef LIMBOAI_MODULE
#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/templates/hash_set.h"
#include "core/templates/vector.h"
#include "core/variant/typed_array.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

class GOAPPlanner : public RefCounted {
	GDCLASS(GOAPPlanner, RefCounted);

private:
	// Planning configuration
	int max_iterations = 1000;

	// Planning statistics (from last plan() call)
	int last_iterations = 0;
	double last_plan_time_ms = 0.0;

	// Internal A* node structure
	struct PlannerNode {
		Ref<GOAPWorldState> state;
		Ref<GOAPAction> action; // Action that led to this state (null for start node)
		int g_cost = 0;         // Cost from start
		int h_cost = 0;         // Heuristic cost to goal
		int f_cost() const { return g_cost + h_cost; }
		int parent_index = -1;  // Index of parent node in closed list
		uint32_t state_hash = 0; // Cached hash for quick comparison
		bool in_closed = false;  // For lazy deletion from open list
	};

	// Min-heap entry: stores node index and f_cost for priority ordering
	struct HeapEntry {
		int node_index;
		int f_cost;
		bool operator>(const HeapEntry &other) const { return f_cost > other.f_cost; }
	};

	// Binary min-heap operations for O(log n) priority queue
	static void _heap_push(Vector<HeapEntry> &heap, HeapEntry entry);
	static HeapEntry _heap_pop(Vector<HeapEntry> &heap);
	static void _heap_sift_up(Vector<HeapEntry> &heap, int index);
	static void _heap_sift_down(Vector<HeapEntry> &heap, int index);

	// Helper to reconstruct plan from closed list
	TypedArray<GOAPAction> _reconstruct_plan(const Vector<PlannerNode> &p_closed_list, int p_goal_index) const;

protected:
	static void _bind_methods();

public:
	void set_max_iterations(int p_max) { max_iterations = p_max; }
	int get_max_iterations() const { return max_iterations; }

	int get_last_iterations() const { return last_iterations; }
	double get_last_plan_time_ms() const { return last_plan_time_ms; }

	// Main planning function
	// Returns array of GOAPActions in execution order (first action first)
	// Returns empty array if no plan found
	TypedArray<GOAPAction> plan(
			const TypedArray<GOAPAction> &p_available_actions,
			const Ref<GOAPWorldState> &p_current_state,
			const Ref<GOAPGoal> &p_goal,
			Node *p_agent = nullptr,
			const Ref<Blackboard> &p_blackboard = Ref<Blackboard>());

	// Convenience overload using goal world state directly
	TypedArray<GOAPAction> plan_to_state(
			const TypedArray<GOAPAction> &p_available_actions,
			const Ref<GOAPWorldState> &p_current_state,
			const Ref<GOAPWorldState> &p_goal_state,
			Node *p_agent = nullptr,
			const Ref<Blackboard> &p_blackboard = Ref<Blackboard>());

	GOAPPlanner() {}
};

#endif // GOAP_PLANNER_H
