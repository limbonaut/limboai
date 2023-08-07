/**
 * bt_set_var.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_set_var.h"

#include "modules/limboai/util/limbo_utility.h"

#include "core/variant/callable.h"

String BTSetVar::_generate_name() const {
	if (variable.is_empty()) {
		return "SetVar ???";
	}
	return vformat("SetVar %s = %s", LimboUtility::get_singleton()->decorate_var(variable),
			value.is_valid() ? Variant(value) : Variant("???"));
}

int BTSetVar::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(variable.is_empty(), FAILURE, "BBSetVar: `variable` is not set.");
	ERR_FAIL_COND_V_MSG(!value.is_valid(), FAILURE, "BBSetVar: `value` is not set.");
	get_blackboard()->set_var(variable, value->get_value(get_agent(), get_blackboard()));
	return SUCCESS;
};

void BTSetVar::set_variable(const String &p_variable) {
	variable = p_variable;
	emit_changed();
}

void BTSetVar::set_value(Ref<BBVariant> p_value) {
	value = p_value;
	emit_changed();
	if (Engine::get_singleton()->is_editor_hint() && value.is_valid()) {
		value->connect(SNAME("changed"), Callable(this, SNAME("emit_changed")));
	}
}

String BTSetVar::get_configuration_warning() const {
	String warning = BTAction::get_configuration_warning();
	if (!warning.is_empty()) {
		warning += "\n";
	}
	if (variable.is_empty()) {
		warning += "`variable` should be assigned.\n";
	}
	if (!value.is_valid()) {
		warning += "`value` should be assigned.\n";
	}
	return warning;
}

void BTSetVar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_variable", "p_variable"), &BTSetVar::set_variable);
	ClassDB::bind_method(D_METHOD("get_variable"), &BTSetVar::get_variable);
	ClassDB::bind_method(D_METHOD("set_value", "p_value"), &BTSetVar::set_value);
	ClassDB::bind_method(D_METHOD("get_value"), &BTSetVar::get_value);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "variable"), "set_variable", "get_variable");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "value", PROPERTY_HINT_RESOURCE_TYPE, "BBVariant"), "set_value", "get_value");
}
