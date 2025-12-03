/**
 * goap_goal.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "goap_goal.h"

Ref<GOAPWorldState> GOAPGoal::create_world_state() const {
	Ref<GOAPWorldState> world_state;
	world_state.instantiate();
	world_state->set_state(target_state.duplicate());
	return world_state;
}

bool GOAPGoal::is_satisfied(const Ref<GOAPWorldState> &p_world_state) const {
	ERR_FAIL_COND_V(p_world_state.is_null(), false);
	Ref<GOAPWorldState> goal_state = create_world_state();
	return p_world_state->satisfies(goal_state);
}

TypedArray<StringName> GOAPGoal::get_required_facts() const {
	return target_state.keys();
}

void GOAPGoal::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_goal_name", "name"), &GOAPGoal::set_goal_name);
	ClassDB::bind_method(D_METHOD("get_goal_name"), &GOAPGoal::get_goal_name);

	ClassDB::bind_method(D_METHOD("set_target_state", "state"), &GOAPGoal::set_target_state);
	ClassDB::bind_method(D_METHOD("get_target_state"), &GOAPGoal::get_target_state);

	ClassDB::bind_method(D_METHOD("set_priority", "priority"), &GOAPGoal::set_priority);
	ClassDB::bind_method(D_METHOD("get_priority"), &GOAPGoal::get_priority);

	ClassDB::bind_method(D_METHOD("create_world_state"), &GOAPGoal::create_world_state);
	ClassDB::bind_method(D_METHOD("is_satisfied", "world_state"), &GOAPGoal::is_satisfied);
	ClassDB::bind_method(D_METHOD("get_required_facts"), &GOAPGoal::get_required_facts);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "goal_name"), "set_goal_name", "get_goal_name");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "target_state"), "set_target_state", "get_target_state");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "priority"), "set_priority", "get_priority");
}
