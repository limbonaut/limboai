/**
 * bt_set_agent_property.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_set_agent_property.h"

void BTSetAgentProperty::set_property(StringName p_prop) {
	property = p_prop;
	emit_changed();
}

void BTSetAgentProperty::set_value(Ref<BBVariant> p_value) {
	value = p_value;
	emit_changed();
	if (Engine::get_singleton()->is_editor_hint() && value.is_valid()) {
		value->connect(SNAME("changed"), Callable(this, SNAME("emit_changed")));
	}
}

PackedStringArray BTSetAgentProperty::get_configuration_warnings() const {
	PackedStringArray warnings = BTAction::get_configuration_warnings();
	if (property == StringName()) {
		warnings.append("`property` should be assigned.");
	}
	if (!value.is_valid()) {
		warnings.append("`value` should be assigned.");
	}
	return warnings;
}

String BTSetAgentProperty::_generate_name() const {
	if (property == StringName()) {
		return "SetAgentProperty ???";
	}

	return vformat("Set agent.%s = %s", property,
			value.is_valid() ? Variant(value) : Variant("???"));
}

int BTSetAgentProperty::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(property == StringName(), FAILURE, "BTSetAgentProperty: `property` is not set.");
	ERR_FAIL_COND_V_MSG(!value.is_valid(), FAILURE, "BTSetAgentProperty: `value` is not set.");

	StringName error_value = SNAME("ErrorGettingValue");
	Variant v = value->get_value(get_agent(), get_blackboard(), error_value);
	ERR_FAIL_COND_V_MSG(v == Variant(error_value), FAILURE, "BTSetAgentProperty: Couldn't get value of value-parameter.");

	bool r_valid;
	get_agent()->set(property, v, &r_valid);
	ERR_FAIL_COND_V_MSG(!r_valid, FAILURE, vformat("BTSetAgentProperty: Couldn't set property \"%s\" with value \"%s\"", property, v));
	return SUCCESS;
}

void BTSetAgentProperty::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_property", "p_property"), &BTSetAgentProperty::set_property);
	ClassDB::bind_method(D_METHOD("get_property"), &BTSetAgentProperty::get_property);
	ClassDB::bind_method(D_METHOD("set_value", "p_value"), &BTSetAgentProperty::set_value);
	ClassDB::bind_method(D_METHOD("get_value"), &BTSetAgentProperty::get_value);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "property"), "set_property", "get_property");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "value", PROPERTY_HINT_RESOURCE_TYPE, "BBVariant"), "set_value", "get_value");
}
