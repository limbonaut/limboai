/* blackboard.cpp */

#include "blackboard.h"

Variant Blackboard::get_var(const Variant &p_key, const Variant &p_default) const {
	if (data.has(p_key)) {
		return data.get_valid(p_key);
	} else if (parent.is_valid()) {
		return parent->get_var(p_key, p_default);
	} else {
		return p_default;
	}
}

void Blackboard::set_var(const Variant &p_key, const Variant &p_value) {
	data[p_key] = p_value;
}

bool Blackboard::has_var(const Variant &p_key) const {
	return data.has(p_key) || (parent.is_valid() && parent->has_var(p_key));
}

void Blackboard::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_data"), &Blackboard::get_data);
	ClassDB::bind_method(D_METHOD("set_data", "p_data"), &Blackboard::set_data);
	ClassDB::bind_method(D_METHOD("get_var", "p_key", "p_default"), &Blackboard::get_var, Variant());
	ClassDB::bind_method(D_METHOD("set_var", "p_key", "p_value"), &Blackboard::set_var);
	ClassDB::bind_method(D_METHOD("has_var", "p_key"), &Blackboard::has_var);
	ClassDB::bind_method(D_METHOD("set_parent_scope", "p_blackboard"), &Blackboard::set_parent_scope);
	ClassDB::bind_method(D_METHOD("get_parent_scope"), &Blackboard::get_parent_scope);
}