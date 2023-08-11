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

void BTSetAgentProperty::set_property_name(StringName p_prop) {
	property_name = p_prop;
	emit_changed();
}

void BTSetAgentProperty::set_value(Ref<BBVariant> p_value) {
	value = p_value;
	emit_changed();
	if (Engine::get_singleton()->is_editor_hint() && value.is_valid()) {
		value->connect(SNAME("changed"), Callable(this, SNAME("emit_changed")));
	}
}

String BTSetAgentProperty::get_configuration_warning() const {
	String warning = BTAction::get_configuration_warning();
	if (!warning.is_empty()) {
		warning += "\n";
	}
	if (property_name == StringName()) {
		warning += "`property_name` should be assigned.\n";
	}
	if (!value.is_valid()) {
		warning += "`value` should be assigned.\n";
	}
	return warning;
}

String BTSetAgentProperty::_generate_name() const {
	if (property_name == StringName()) {
		return "SetAgentProperty ???";
	}

	return vformat("Set agent.%s = %s", property_name,
			value.is_valid() ? Variant(value) : Variant("???"));
}

int BTSetAgentProperty::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(property_name == StringName(), FAILURE, "BTSetAgentProperty: `property_name` is not set.");
	ERR_FAIL_COND_V_MSG(!value.is_valid(), FAILURE, "BTSetAgentProperty: `value` is not set.");

	bool r_valid;
	get_agent()->set(property_name, value->get_value(get_agent(), get_blackboard()), &r_valid);
	ERR_FAIL_COND_V_MSG(!r_valid, FAILURE, vformat("BTSetAgentProperty: Agent doesn't have property named \"%s\"", property_name));
	return SUCCESS;
}

void BTSetAgentProperty::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_property_name", "p_property_name"), &BTSetAgentProperty::set_property_name);
	ClassDB::bind_method(D_METHOD("get_property_name"), &BTSetAgentProperty::get_property_name);
	ClassDB::bind_method(D_METHOD("set_value", "p_value"), &BTSetAgentProperty::set_value);
	ClassDB::bind_method(D_METHOD("get_value"), &BTSetAgentProperty::get_value);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "property_name"), "set_property_name", "get_property_name");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "value", PROPERTY_HINT_RESOURCE_TYPE, "BBVariant"), "set_value", "get_value");
}
