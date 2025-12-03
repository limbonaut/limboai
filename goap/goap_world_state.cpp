/**
 * goap_world_state.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "goap_world_state.h"

#ifdef LIMBOAI_MODULE
#include "core/math/math_funcs.h"
#include "core/variant/variant.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/variant/utility_functions.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

bool GOAPWorldState::_values_match(const Variant &p_a, const Variant &p_b) {
	if (p_a.get_type() != p_b.get_type()) {
		return false;
	}

	switch (p_a.get_type()) {
		case Variant::FLOAT: {
			double a = p_a;
			double b = p_b;
			return Math::abs(a - b) < 0.001;
		}
		default:
			return p_a == p_b;
	}
}

void GOAPWorldState::set_fact(const StringName &p_name, const Variant &p_value) {
	state[p_name] = p_value;
}

Variant GOAPWorldState::get_fact(const StringName &p_name, const Variant &p_default) const {
	if (state.has(p_name)) {
		return state[p_name];
	}
	return p_default;
}

bool GOAPWorldState::has_fact(const StringName &p_name) const {
	return state.has(p_name);
}

void GOAPWorldState::erase_fact(const StringName &p_name) {
	state.erase(p_name);
}

void GOAPWorldState::clear() {
	state.clear();
}

TypedArray<StringName> GOAPWorldState::get_fact_names() const {
	return state.keys();
}

void GOAPWorldState::populate_from_blackboard(const Ref<Blackboard> &p_blackboard, const TypedArray<StringName> &p_fact_names) {
	ERR_FAIL_COND(p_blackboard.is_null());
	for (int i = 0; i < p_fact_names.size(); i++) {
		StringName fact_name = p_fact_names[i];
		if (p_blackboard->has_var(fact_name)) {
			state[fact_name] = p_blackboard->get_var(fact_name, Variant(), false);
		}
	}
}

bool GOAPWorldState::satisfies(const Ref<GOAPWorldState> &p_goal) const {
	ERR_FAIL_COND_V(p_goal.is_null(), false);

	Array goal_keys = p_goal->state.keys();
	for (int i = 0; i < goal_keys.size(); i++) {
		StringName key = goal_keys[i];
		if (!state.has(key)) {
			return false;
		}
		if (!_values_match(state[key], p_goal->state[key])) {
			return false;
		}
	}
	return true;
}

int GOAPWorldState::distance_to(const Ref<GOAPWorldState> &p_goal) const {
	ERR_FAIL_COND_V(p_goal.is_null(), INT_MAX);

	int distance = 0;
	Array goal_keys = p_goal->state.keys();
	for (int i = 0; i < goal_keys.size(); i++) {
		StringName key = goal_keys[i];
		Variant goal_value = p_goal->state[key];
		Variant current_value = state.has(key) ? state[key] : Variant();

		if (!_values_match(current_value, goal_value)) {
			distance += 1;
		}
	}
	return distance;
}

Ref<GOAPWorldState> GOAPWorldState::duplicate() const {
	Ref<GOAPWorldState> new_state;
	new_state.instantiate();
	new_state->state = state.duplicate();
	return new_state;
}

Ref<GOAPWorldState> GOAPWorldState::apply_effects(const Dictionary &p_effects) const {
	Ref<GOAPWorldState> new_state = duplicate();
	Array effect_keys = p_effects.keys();
	for (int i = 0; i < effect_keys.size(); i++) {
		StringName key = effect_keys[i];
		new_state->state[key] = p_effects[key];
	}
	return new_state;
}

uint32_t GOAPWorldState::compute_hash() const {
	// Compute a hash of all fact-value pairs
	// We need a stable hash that doesn't depend on iteration order
	uint32_t hash = 0;
	Array keys = state.keys();
	for (int i = 0; i < keys.size(); i++) {
		StringName key = keys[i];
		Variant value = state[key];
		// XOR individual hashes so order doesn't matter
		uint32_t key_hash = key.hash();
		uint32_t value_hash = value.hash();
		// Combine key and value hash, then XOR into result
		hash ^= key_hash * 31 + value_hash;
	}
	return hash;
}

bool GOAPWorldState::equals(const Ref<GOAPWorldState> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	// Both must satisfy each other for equality
	return satisfies(p_other) && p_other->satisfies(Ref<GOAPWorldState>(const_cast<GOAPWorldState *>(this)));
}

void GOAPWorldState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_fact", "name", "value"), &GOAPWorldState::set_fact);
	ClassDB::bind_method(D_METHOD("get_fact", "name", "default"), &GOAPWorldState::get_fact, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("has_fact", "name"), &GOAPWorldState::has_fact);
	ClassDB::bind_method(D_METHOD("erase_fact", "name"), &GOAPWorldState::erase_fact);
	ClassDB::bind_method(D_METHOD("clear"), &GOAPWorldState::clear);

	ClassDB::bind_method(D_METHOD("set_state", "state"), &GOAPWorldState::set_state);
	ClassDB::bind_method(D_METHOD("get_state"), &GOAPWorldState::get_state);

	ClassDB::bind_method(D_METHOD("get_fact_names"), &GOAPWorldState::get_fact_names);
	ClassDB::bind_method(D_METHOD("populate_from_blackboard", "blackboard", "fact_names"), &GOAPWorldState::populate_from_blackboard);

	ClassDB::bind_method(D_METHOD("satisfies", "goal"), &GOAPWorldState::satisfies);
	ClassDB::bind_method(D_METHOD("distance_to", "goal"), &GOAPWorldState::distance_to);
	ClassDB::bind_method(D_METHOD("duplicate"), &GOAPWorldState::duplicate);
	ClassDB::bind_method(D_METHOD("apply_effects", "effects"), &GOAPWorldState::apply_effects);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "state"), "set_state", "get_state");
}
