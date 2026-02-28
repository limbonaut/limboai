/**
 * blackboard.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "blackboard.h"
#include "../compat/print.h"

#ifdef LIMBOAI_MODULE
#include "core/config/engine.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#endif // LIMBOAI_GDEXTENSION

bool Blackboard::_set(const StringName &p_name, const Variant &p_value) {
	// Read-only for now.
	return false;
}

bool Blackboard::_get(const StringName &p_name, Variant &r_ret) const {
	String name_str = p_name;
	if (!name_str.begins_with("scope_")) {
		return false;
	}

	// Parse "scope_N/var_name".
	String scope_part = name_str.get_slicec('/', 0);
	int scope_idx = scope_part.substr(6).to_int(); // Skip "scope_".
	String var_name = name_str.substr(scope_part.length() + 1);

	// Walk to the target scope.
	const Blackboard *bb = this;
	for (int i = 0; i < scope_idx && bb; i++) {
		bb = bb->parent.ptr();
	}

	if (!bb || !bb->data.has(var_name)) {
		return false;
	}

	r_ret = bb->data.get(var_name).get_value();
	return true;
}

void Blackboard::_get_property_list(List<PropertyInfo> *p_list) const {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	// Collect scopes: [0]=this (local), [1]=parent, [2]=grandparent, ...
	Vector<const Blackboard *> scopes;
	const Blackboard *bb = this;
	while (bb) {
		scopes.push_back(bb);
		bb = bb->parent.ptr();
	}

	// Emit in reverse order: top-most scope first, local scope last.
	for (int i = scopes.size() - 1; i >= 0; i--) {
		String scope_prefix = "scope_" + itos(i) + "/";
		String group_name = "Scope " + itos(i);
		if (i == 0) {
			group_name += " (local)";
		} else {
			group_name += " (parent)";
		}
		p_list->push_back(PropertyInfo(Variant::NIL, group_name, PROPERTY_HINT_NONE, scope_prefix, PROPERTY_USAGE_GROUP));

		// Collect and sort variable names.
		Vector<String> sorted_names;
		for (const KeyValue<StringName, BBVariable> &kv : scopes[i]->data) {
			sorted_names.push_back(kv.key);
		}
		sorted_names.sort();

		for (const String &var_name : sorted_names) {
			const BBVariable &var = scopes[i]->data.get(var_name);
			p_list->push_back(PropertyInfo(var.get_type(), scope_prefix + var_name, var.get_hint(), var.get_hint_string(), PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
		}
	}
}

Ref<Blackboard> Blackboard::top() const {
	Ref<Blackboard> bb(this);
	while (bb->get_parent().is_valid()) {
		bb = bb->get_parent();
	}
	return bb;
}

Variant Blackboard::get_var(const StringName &p_name, const Variant &p_default, bool p_complain) const {
	if (data.has(p_name)) {
		return data.get(p_name).get_value();
	} else if (parent.is_valid()) {
		return parent->get_var(p_name, p_default, p_complain);
	} else {
		if (p_complain) {
			ERR_PRINT(vformat("Blackboard: Variable \"%s\" not found.", p_name));
		}
		return p_default;
	}
}

void Blackboard::set_var(const StringName &p_name, const Variant &p_value) {
	if (data.has(p_name)) {
		// Not checking type - allowing duck-typing.
		data[p_name].set_value(p_value);
	} else {
		BBVariable var(p_value.get_type());
		var.set_value(p_value);
		data.insert(p_name, var);
	}
}

bool Blackboard::has_var(const StringName &p_name) const {
	return data.has(p_name) || (parent.is_valid() && parent->has_var(p_name));
}

void Blackboard::erase_var(const StringName &p_name) {
	data.erase(p_name);
}

TypedArray<StringName> Blackboard::list_vars() const {
	TypedArray<StringName> var_names;
	var_names.resize(data.size());
	int idx = 0;
	for (const KeyValue<StringName, BBVariable> &kv : data) {
		var_names[idx] = kv.key;
		idx += 1;
	}
	return var_names;
}

void Blackboard::print_state() const {
	Ref<Blackboard> bb{ this };
	int scope_idx = 0;
	while (bb.is_valid()) {
		int i = 0;
		String line = "Scope " + itos(scope_idx) + ": { ";
		for (const KeyValue<StringName, BBVariable> &kv : bb->data) {
			if (i > 0) {
				line += ", ";
			}
			line += String(kv.key) + ": " + String(kv.value.get_value());
			i++;
		}
		line += " }";
		PRINT_LINE(line);
		bb = bb->get_parent();
		scope_idx++;
	}
}

Dictionary Blackboard::get_vars_as_dict() const {
	Dictionary dict;
	for (const KeyValue<StringName, BBVariable> &kv : data) {
		dict[kv.key] = kv.value.get_value();
	}
	return dict;
}

void Blackboard::populate_from_dict(const Dictionary &p_dictionary) {
	Array keys = p_dictionary.keys();
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i].get_type() == Variant::STRING_NAME || keys[i].get_type() == Variant::STRING) {
			set_var(keys[i], p_dictionary[keys[i]]);
		} else {
			ERR_PRINT("Blackboard: Invalid key type in dictionary to populate blackboard. Must be StringName or String.");
		}
	}
}

void Blackboard::bind_var_to_property(const StringName &p_name, Object *p_object, const StringName &p_property, bool p_create) {
	if (!data.has(p_name)) {
		if (p_create) {
			data.insert(p_name, BBVariable());
		} else {
			ERR_FAIL_MSG("Blackboard: Can't bind variable that doesn't exist (var: " + p_name + ").");
		}
	}
	data[p_name].bind(p_object, p_property);
}

void Blackboard::unbind_var(const StringName &p_name) {
	ERR_FAIL_COND_MSG(!data.has(p_name), "Blackboard: Can't unbind variable that doesn't exist (var: " + p_name + ").");
	data[p_name].unbind();
}

void Blackboard::assign_var(const StringName &p_name, const BBVariable &p_var) {
	data.insert(p_name, p_var);
}

void Blackboard::link_var(const StringName &p_name, const Ref<Blackboard> &p_target_blackboard, const StringName &p_target_var, bool p_create) {
	if (!data.has(p_name)) {
		if (p_create) {
			data.insert(p_name, BBVariable());
		} else {
			ERR_FAIL_MSG("Blackboard: Can't link variable that doesn't exist (var: " + p_name + ").");
		}
	}
	ERR_FAIL_COND_MSG(p_target_blackboard.is_null(), "Blackboard: Can't link variable to target blackboard that is null (var: " + p_name + ").");
	ERR_FAIL_COND_MSG(!p_target_blackboard->data.has(p_target_var), "Blackboard: Can't link variable to non-existent target (var: " + p_name + ", target: " + p_target_var + ").");
	data[p_name] = p_target_blackboard->data[p_target_var];
}

void Blackboard::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_var", "var_name", "default", "complain"), &Blackboard::get_var, DEFVAL(Variant()), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("set_var", "var_name", "value"), &Blackboard::set_var);
	ClassDB::bind_method(D_METHOD("has_var", "var_name"), &Blackboard::has_var);
	ClassDB::bind_method(D_METHOD("set_parent", "blackboard"), &Blackboard::set_parent);
	ClassDB::bind_method(D_METHOD("get_parent"), &Blackboard::get_parent);
	ClassDB::bind_method(D_METHOD("erase_var", "var_name"), &Blackboard::erase_var);
	ClassDB::bind_method(D_METHOD("clear"), &Blackboard::clear);
	ClassDB::bind_method(D_METHOD("list_vars"), &Blackboard::list_vars);
	ClassDB::bind_method(D_METHOD("print_state"), &Blackboard::print_state);
	ClassDB::bind_method(D_METHOD("get_vars_as_dict"), &Blackboard::get_vars_as_dict);
	ClassDB::bind_method(D_METHOD("populate_from_dict", "dictionary"), &Blackboard::populate_from_dict);
	ClassDB::bind_method(D_METHOD("top"), &Blackboard::top);
	ClassDB::bind_method(D_METHOD("bind_var_to_property", "var_name", "object", "property", "create"), &Blackboard::bind_var_to_property, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("unbind_var", "var_name"), &Blackboard::unbind_var);
	ClassDB::bind_method(D_METHOD("link_var", "var_name", "target_blackboard", "target_var", "create"), &Blackboard::link_var, DEFVAL(false));
}
