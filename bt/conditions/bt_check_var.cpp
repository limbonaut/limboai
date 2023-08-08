/**
 * bt_check_var.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_check_var.h"

#include "modules/limboai/util/limbo_utility.h"

#include "core/variant/callable.h"

VARIANT_ENUM_CAST(BTCheckVar::CheckType);

void BTCheckVar::set_variable(String p_variable) {
	variable = p_variable;
	emit_changed();
}

void BTCheckVar::set_check_type(CheckType p_check_type) {
	check_type = p_check_type;
	emit_changed();
}

void BTCheckVar::set_value(Ref<BBVariant> p_value) {
	value = p_value;
	emit_changed();
	if (Engine::get_singleton()->is_editor_hint() && value.is_valid()) {
		value->connect(SNAME("changed"), Callable(this, SNAME("emit_changed")));
	}
}

String BTCheckVar::get_configuration_warning() const {
	String warning = BTCondition::get_configuration_warning();
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

String BTCheckVar::_generate_name() const {
	if (variable.is_empty()) {
		return "CheckVar ???";
	}

	String check_str = "?";
	switch (check_type) {
		case CheckType::CHECK_EQUAL: {
			check_str = "==";
		} break;
		case CheckType::CHECK_LESS_THAN: {
			check_str = "<";
		} break;
		case CheckType::CHECK_LESS_THAN_OR_EQUAL: {
			check_str = "<=";
		} break;
		case CheckType::CHECK_GREATER_THAN: {
			check_str = ">";
		} break;
		case CheckType::CHECK_GREATER_THAN_OR_EQUAL: {
			check_str = ">=";
		} break;
		case CheckType::CHECK_NOT_EQUAL: {
			check_str = "!=";
		} break;
	}

	return vformat("Check if %s %s %s", LimboUtility::get_singleton()->decorate_var(variable), check_str,
			value.is_valid() ? Variant(value) : Variant("???"));
}

int BTCheckVar::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(variable.is_empty(), FAILURE, "BBCheckVar: `variable` is not set.");
	ERR_FAIL_COND_V_MSG(!value.is_valid(), FAILURE, "BBCheckVar: `value` is not set.");

	ERR_FAIL_COND_V_MSG(!get_blackboard()->has_var(variable), FAILURE, vformat("BBCheckVar: Blackboard variable doesn't exist: \"%s\". Returning FAILURE.", variable));

	Variant left_value = get_blackboard()->get_var(variable, Variant());
	Variant right_value = value->get_value(get_agent(), get_blackboard());

	switch (check_type) {
		case CheckType::CHECK_EQUAL: {
			return Variant::evaluate(Variant::OP_EQUAL, left_value, right_value) ? SUCCESS : FAILURE;
		} break;
		case CheckType::CHECK_LESS_THAN: {
			return Variant::evaluate(Variant::OP_LESS, left_value, right_value) ? SUCCESS : FAILURE;
		} break;
		case CheckType::CHECK_LESS_THAN_OR_EQUAL: {
			return Variant::evaluate(Variant::OP_LESS_EQUAL, left_value, right_value) ? SUCCESS : FAILURE;
		} break;
		case CheckType::CHECK_GREATER_THAN: {
			return Variant::evaluate(Variant::OP_GREATER, left_value, right_value) ? SUCCESS : FAILURE;
		} break;
		case CheckType::CHECK_GREATER_THAN_OR_EQUAL: {
			return Variant::evaluate(Variant::OP_GREATER_EQUAL, left_value, right_value) ? SUCCESS : FAILURE;
		} break;
		case CheckType::CHECK_NOT_EQUAL: {
			return Variant::evaluate(Variant::OP_NOT_EQUAL, left_value, right_value) ? SUCCESS : FAILURE;
		} break;
		default: {
			return FAILURE;
		} break;
	}
}

void BTCheckVar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_variable", "p_variable"), &BTCheckVar::set_variable);
	ClassDB::bind_method(D_METHOD("get_variable"), &BTCheckVar::get_variable);
	ClassDB::bind_method(D_METHOD("set_check_type", "p_check_type"), &BTCheckVar::set_check_type);
	ClassDB::bind_method(D_METHOD("get_check_type"), &BTCheckVar::get_check_type);
	ClassDB::bind_method(D_METHOD("set_value", "p_value"), &BTCheckVar::set_value);
	ClassDB::bind_method(D_METHOD("get_value"), &BTCheckVar::get_value);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "variable"), "set_variable", "get_variable");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "check_type", PROPERTY_HINT_ENUM, "Equal,Less Than,Less Than Or Equal,Greater Than,Greater Than Or Equal,Not Equal"), "set_check_type", "get_check_type");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "value", PROPERTY_HINT_RESOURCE_TYPE, "BBVariant"), "set_value", "get_value");

	BIND_ENUM_CONSTANT(CHECK_EQUAL);
	BIND_ENUM_CONSTANT(CHECK_LESS_THAN);
	BIND_ENUM_CONSTANT(CHECK_LESS_THAN_OR_EQUAL);
	BIND_ENUM_CONSTANT(CHECK_GREATER_THAN);
	BIND_ENUM_CONSTANT(CHECK_GREATER_THAN_OR_EQUAL);
	BIND_ENUM_CONSTANT(CHECK_NOT_EQUAL);
}
