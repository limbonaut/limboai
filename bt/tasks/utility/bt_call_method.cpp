/**
 * bt_call_method.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_call_method.h"

#include "../../../util/limbo_compat.h"

//**** Setters / Getters

void BTCallMethod::set_method(StringName p_method_name) {
	method = p_method_name;
	emit_changed();
}

void BTCallMethod::set_node_param(Ref<BBNode> p_object) {
	node_param = p_object;
	emit_changed();
	if (Engine::get_singleton()->is_editor_hint() && node_param.is_valid()) {
		node_param->connect(LW_NAME(changed), Callable(this, LW_NAME(emit_changed)));
	}
}

void BTCallMethod::set_include_delta(bool p_include_delta) {
	include_delta = p_include_delta;
	emit_changed();
}

void BTCallMethod::set_args(Array p_args) {
	args = p_args;
	emit_changed();
}

//**** Task Implementation

PackedStringArray BTCallMethod::get_configuration_warnings() {
	PackedStringArray warnings = BTAction::get_configuration_warnings();
	if (method == StringName()) {
		warnings.append("Method Name is not set.");
	}
	if (node_param.is_null()) {
		warnings.append("Node parameter is not set.");
	} else if (node_param->get_value_source() == BBParam::SAVED_VALUE && node_param->get_saved_value() == Variant()) {
		warnings.append("Path to node is not set.");
	} else if (node_param->get_value_source() == BBParam::BLACKBOARD_VAR && node_param->get_variable() == StringName()) {
		warnings.append("Node blackboard variable is not set.");
	}
	return warnings;
}

String BTCallMethod::_generate_name() {
	String args_str = include_delta ? "delta" : "";
	if (args.size() > 0) {
		if (!args_str.is_empty()) {
			args_str += ", ";
		}
		args_str += vformat("%s", args).trim_prefix("[").trim_suffix("]");
	}
	return vformat("CallMethod %s(%s)  node: %s",
			(method != StringName() ? method : "???"),
			args_str,
			(node_param.is_valid() && !node_param->to_string().is_empty() ? node_param->to_string() : "???"));
}

BT::Status BTCallMethod::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(method == StringName(), FAILURE, "BTCallMethod: Method Name is not set.");
	ERR_FAIL_COND_V_MSG(node_param.is_null(), FAILURE, "BTCallMethod: Node parameter is not set.");
	Object *obj = node_param->get_value(get_agent(), get_blackboard());
	ERR_FAIL_COND_V_MSG(obj == nullptr, FAILURE, "BTCallMethod: Failed to get object: " + node_param->to_string());

#ifdef LIMBOAI_MODULE
	const Variant delta = include_delta ? Variant(p_delta) : Variant();
	const Variant **argptrs = nullptr;

	int argument_count = include_delta ? args.size() + 1 : args.size();
	if (argument_count > 0) {
		argptrs = (const Variant **)alloca(sizeof(Variant *) * argument_count);
		if (include_delta) {
			argptrs[0] = &delta;
		}
		for (int i = 0; i < args.size(); i++) {
			argptrs[i + int(include_delta)] = &args[i];
		}
	}

	Callable::CallError ce;
	obj->callp(method, argptrs, argument_count, ce);
	if (ce.error != Callable::CallError::CALL_OK) {
		ERR_FAIL_V_MSG(FAILURE, "BTCallMethod: Error calling method: " + Variant::get_call_error_text(obj, method, argptrs, argument_count, ce) + ".");
	}
#elif LIMBOAI_GDEXTENSION
	Array call_args;
	if (include_delta) {
		call_args.push_back(Variant(p_delta));
		call_args.append_array(args);
	} else {
		call_args = args;
	}

	// TODO: Unsure how to detect call error, so we return SUCCESS for now...
	obj->callv(method, call_args);
#endif // LIMBOAI_MODULE & LIMBOAI_GDEXTENSION

	return SUCCESS;
}

//**** Godot

void BTCallMethod::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_method", "p_method"), &BTCallMethod::set_method);
	ClassDB::bind_method(D_METHOD("get_method"), &BTCallMethod::get_method);
	ClassDB::bind_method(D_METHOD("set_node_param", "p_param"), &BTCallMethod::set_node_param);
	ClassDB::bind_method(D_METHOD("get_node_param"), &BTCallMethod::get_node_param);
	ClassDB::bind_method(D_METHOD("set_args", "p_args"), &BTCallMethod::set_args);
	ClassDB::bind_method(D_METHOD("get_args"), &BTCallMethod::get_args);
	ClassDB::bind_method(D_METHOD("set_include_delta", "p_include_delta"), &BTCallMethod::set_include_delta);
	ClassDB::bind_method(D_METHOD("is_delta_included"), &BTCallMethod::is_delta_included);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_RESOURCE_TYPE, "BBNode"), "set_node_param", "get_node_param");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "method"), "set_method", "get_method");
	ADD_GROUP("Arguments", "args_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "args_include_delta"), "set_include_delta", "is_delta_included");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "args"), "set_args", "get_args");

	// ADD_PROPERTY_DEFAULT("args_include_delta", false);
}

BTCallMethod::BTCallMethod() {
}
