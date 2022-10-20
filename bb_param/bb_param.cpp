/* bb_param.cpp */

#include "bb_param.h"
#include "core/class_db.h"
#include "core/error_macros.h"
#include "core/object.h"
#include "core/variant.h"

VARIANT_ENUM_CAST(BBParam::ValueSource);

void BBParam::set_value_source(ValueSource p_value) {
	value_source = p_value;
	property_list_changed_notify();
	_update_name();
	emit_changed();
}

void BBParam::set_saved_value(Variant p_value) {
	saved_value = p_value;
	_update_name();
	emit_changed();
}

void BBParam::set_variable(const String &p_value) {
	variable = p_value;
	_update_name();
	emit_changed();
}

Variant BBParam::get_value(Object *p_agent, const Ref<Blackboard> &p_blackboard, const Variant &p_default) {
	ERR_FAIL_COND_V(p_blackboard.is_valid(), p_default);

	if (value_source == SAVED_VALUE) {
		return saved_value;
	} else {
		return p_blackboard->get_var(variable, p_default);
	}
}

void BBParam::_get_property_list(List<PropertyInfo> *p_list) const {
	if (value_source == ValueSource::SAVED_VALUE) {
		p_list->push_back(PropertyInfo(get_type(), "saved_value"));
	} else {
		p_list->push_back(PropertyInfo(Variant::STRING, "variable"));
	}
}

void BBParam::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_value_source", "p_value_source"), &BBParam::set_value_source);
	ClassDB::bind_method(D_METHOD("get_value_source"), &BBParam::get_value_source);
	ClassDB::bind_method(D_METHOD("set_saved_value", "p_value"), &BBParam::set_saved_value);
	ClassDB::bind_method(D_METHOD("get_saved_value"), &BBParam::get_saved_value);
	ClassDB::bind_method(D_METHOD("set_variable", "p_variable_name"), &BBParam::set_variable);
	ClassDB::bind_method(D_METHOD("get_variable"), &BBParam::get_variable);
	ClassDB::bind_method(D_METHOD("get_type"), &BBParam::get_type);
	ClassDB::bind_method(D_METHOD("get_value", "p_agent", "p_blackboard", "p_default"), &BBParam::get_value, Variant());

	ADD_PROPERTY(PropertyInfo(Variant::INT, "value_source", PROPERTY_HINT_ENUM, "Saved Value, Blackboard Var"), "set_value_source", "get_value_source");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "variable", PROPERTY_HINT_NONE, "", 0), "set_variable", "get_variable");
	ADD_PROPERTY(PropertyInfo(Variant::NIL, "saved_value", PROPERTY_HINT_NONE, "", 0), "set_saved_value", "get_saved_value");

	BIND_ENUM_CONSTANT(SAVED_VALUE);
	BIND_ENUM_CONSTANT(BLACKBOARD_VAR);
}

BBParam::BBParam() {
	value_source = SAVED_VALUE;
	variable = "";
	saved_value = Variant();
}
