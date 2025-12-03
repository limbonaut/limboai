/**
 * goap_action.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "goap_action.h"

bool GOAPAction::is_valid(const Ref<GOAPWorldState> &p_state) const {
	ERR_FAIL_COND_V(p_state.is_null(), false);

	Array precondition_keys = preconditions.keys();
	for (int i = 0; i < precondition_keys.size(); i++) {
		StringName key = precondition_keys[i];
		Variant required_value = preconditions[key];
		Variant current_value = p_state->get_fact(key, Variant());

		// Type mismatch = not valid
		if (current_value.get_type() != required_value.get_type()) {
			return false;
		}

		// Value mismatch = not valid
		if (current_value != required_value) {
			return false;
		}
	}
	return true;
}

Ref<GOAPWorldState> GOAPAction::apply_effects_to_state(const Ref<GOAPWorldState> &p_state) const {
	ERR_FAIL_COND_V(p_state.is_null(), Ref<GOAPWorldState>());
	return p_state->apply_effects(effects);
}

int GOAPAction::get_cost(Node *p_agent, const Ref<Blackboard> &p_blackboard) const {
	int cost = base_cost;

	// Try calling the virtual method for dynamic cost
	if (p_agent != nullptr && p_blackboard.is_valid()) {
		int dynamic_cost;
		if (GDVIRTUAL_CALL(_get_dynamic_cost, p_agent, p_blackboard, base_cost, dynamic_cost)) {
			cost = dynamic_cost;
		}
	}

	return cost;
}

bool GOAPAction::check_procedural_preconditions(Node *p_agent, const Ref<Blackboard> &p_blackboard) const {
	// Try calling the virtual method
	bool result = true;
	if (GDVIRTUAL_CALL(_check_procedural_preconditions, p_agent, p_blackboard, result)) {
		return result;
	}
	// Default: return true (no additional checks)
	return true;
}

TypedArray<StringName> GOAPAction::get_relevant_facts() const {
	TypedArray<StringName> facts;

	// Add precondition facts
	Array precondition_keys = preconditions.keys();
	for (int i = 0; i < precondition_keys.size(); i++) {
		StringName key = precondition_keys[i];
		if (!facts.has(key)) {
			facts.append(key);
		}
	}

	// Add effect facts
	Array effect_keys = effects.keys();
	for (int i = 0; i < effect_keys.size(); i++) {
		StringName key = effect_keys[i];
		if (!facts.has(key)) {
			facts.append(key);
		}
	}

	return facts;
}

bool GOAPAction::produces_fact(const StringName &p_fact_name) const {
	return effects.has(p_fact_name);
}

void GOAPAction::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_action_name", "name"), &GOAPAction::set_action_name);
	ClassDB::bind_method(D_METHOD("get_action_name"), &GOAPAction::get_action_name);

	ClassDB::bind_method(D_METHOD("set_preconditions", "preconditions"), &GOAPAction::set_preconditions);
	ClassDB::bind_method(D_METHOD("get_preconditions"), &GOAPAction::get_preconditions);

	ClassDB::bind_method(D_METHOD("set_effects", "effects"), &GOAPAction::set_effects);
	ClassDB::bind_method(D_METHOD("get_effects"), &GOAPAction::get_effects);

	ClassDB::bind_method(D_METHOD("set_base_cost", "cost"), &GOAPAction::set_base_cost);
	ClassDB::bind_method(D_METHOD("get_base_cost"), &GOAPAction::get_base_cost);

	ClassDB::bind_method(D_METHOD("set_execution_tree", "tree"), &GOAPAction::set_execution_tree);
	ClassDB::bind_method(D_METHOD("get_execution_tree"), &GOAPAction::get_execution_tree);

	ClassDB::bind_method(D_METHOD("is_valid", "state"), &GOAPAction::is_valid);
	ClassDB::bind_method(D_METHOD("apply_effects_to_state", "state"), &GOAPAction::apply_effects_to_state);
	ClassDB::bind_method(D_METHOD("get_cost", "agent", "blackboard"), &GOAPAction::get_cost, DEFVAL(Variant()), DEFVAL(Ref<Blackboard>()));
	ClassDB::bind_method(D_METHOD("check_procedural_preconditions", "agent", "blackboard"), &GOAPAction::check_procedural_preconditions);
	ClassDB::bind_method(D_METHOD("get_relevant_facts"), &GOAPAction::get_relevant_facts);
	ClassDB::bind_method(D_METHOD("produces_fact", "fact_name"), &GOAPAction::produces_fact);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "action_name"), "set_action_name", "get_action_name");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "preconditions"), "set_preconditions", "get_preconditions");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "effects"), "set_effects", "get_effects");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "base_cost", PROPERTY_HINT_RANGE, "1,100,1"), "set_base_cost", "get_base_cost");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "execution_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviorTree"), "set_execution_tree", "get_execution_tree");

	GDVIRTUAL_BIND(_check_procedural_preconditions, "agent", "blackboard");
	GDVIRTUAL_BIND(_get_dynamic_cost, "agent", "blackboard", "base_cost");
}
