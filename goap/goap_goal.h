/**
 * goap_goal.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef GOAP_GOAL_H
#define GOAP_GOAL_H

#include "goap_world_state.h"

#ifdef LIMBOAI_MODULE
#include "core/io/resource.h"
#include "core/variant/dictionary.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

class GOAPGoal : public Resource {
	GDCLASS(GOAPGoal, Resource);

private:
	StringName goal_name;
	Dictionary target_state;
	int priority = 0;

protected:
	static void _bind_methods();

public:
	void set_goal_name(const StringName &p_name) {
		goal_name = p_name;
		emit_changed();
	}
	StringName get_goal_name() const { return goal_name; }

	void set_target_state(const Dictionary &p_state) {
		target_state = p_state;
		emit_changed();
	}
	Dictionary get_target_state() const { return target_state; }

	void set_priority(int p_priority) {
		priority = p_priority;
		emit_changed();
	}
	int get_priority() const { return priority; }

	// Create a GOAPWorldState from the target state
	Ref<GOAPWorldState> create_world_state() const;

	// Check if goal is satisfied by the given world state
	bool is_satisfied(const Ref<GOAPWorldState> &p_world_state) const;

	// Get all fact names required by this goal
	TypedArray<StringName> get_required_facts() const;

	GOAPGoal() {}
};

#endif // GOAP_GOAL_H
