/**
 * blackboard_plan.cpp
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "blackboard_plan.h"

bool BlackboardPlan::_set(const StringName &p_name, const Variant &p_value) {
	String prop_name = p_name;

	// * Editor
	if (data.has(prop_name)) {
		data[prop_name].set_value(p_value);
		return true;
	}

	// * Storage
	if (prop_name.begins_with("var/")) {
		String var_name = prop_name.get_slicec('/', 1);
		String what = prop_name.get_slicec('/', 2);
		if (!data.has(var_name) && what == "name") {
			data.insert(var_name, BBVariable());
		}
		if (what == "name") {
			// We don't store variable name with the variable.
		} else if (what == "type") {
			data[var_name].set_type((Variant::Type)(int)p_value);
		} else if (what == "value") {
			data[var_name].set_value(p_value);
		} else if (what == "hint") {
			data[var_name].set_hint((PropertyHint)(int)p_value);
		} else if (what == "hint_string") {
			data[var_name].set_hint_string(p_value);
		} else {
			return false;
		}
		return true;
	}

	return false;
}

bool BlackboardPlan::_get(const StringName &p_name, Variant &r_ret) const {
	String prop_name = p_name;

	// * Editor
	if (data.has(prop_name)) {
		r_ret = data[prop_name].get_value();
		return true;
	}

	// * Storage
	if (!prop_name.begins_with("var/")) {
		return false;
	}

	String var_name = prop_name.get_slicec('/', 1);
	String what = prop_name.get_slicec('/', 2);
	ERR_FAIL_COND_V(!data.has(var_name), false);
	if (what == "type") {
		r_ret = data[var_name].get_type();
	} else if (what == "value") {
		r_ret = data[var_name].get_value();
	} else if (what == "hint") {
		r_ret = data[var_name].get_hint();
	} else if (what == "hint_string") {
		r_ret = data[var_name].get_hint_string();
	}
	return true;
}

void BlackboardPlan::_get_property_list(List<PropertyInfo> *p_list) const {
	for (const KeyValue<String, BBVariable> &kv : data) {
		String var_name = kv.key;
		BBVariable var = kv.value;

		// * Editor
		if (!is_derived() || !var_name.begins_with("_")) {
			p_list->push_back(PropertyInfo(var.get_type(), var_name, var.get_hint(), var.get_hint_string(), PROPERTY_USAGE_EDITOR));
		}

		// * Storage
		p_list->push_back(PropertyInfo(Variant::STRING, "var/" + var_name + "/name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
		p_list->push_back(PropertyInfo(Variant::INT, "var/" + var_name + "/type", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
		p_list->push_back(PropertyInfo(var.get_type(), "var/" + var_name + "/value", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
		p_list->push_back(PropertyInfo(Variant::INT, "var/" + var_name + "/hint", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
		p_list->push_back(PropertyInfo(Variant::STRING, "var/" + var_name + "/hint_string", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
	}
}

bool BlackboardPlan::_property_can_revert(const StringName &p_name) const {
	return base.is_valid() && base->data.has(p_name);
}

bool BlackboardPlan::_property_get_revert(const StringName &p_name, Variant &r_property) const {
	if (base->data.has(p_name)) {
		r_property = base->data[p_name].get_value();
		return true;
	}
	return false;
}

void BlackboardPlan::set_base_plan(const Ref<BlackboardPlan> &p_base) {
	base = p_base;
	sync_with_base_plan();
}

void BlackboardPlan::set_value(const String &p_name, const Variant &p_value) {
	ERR_FAIL_COND(!data.has(p_name));
	data.get(p_name).set_value(p_value);
	emit_changed();
}

Variant BlackboardPlan::get_value(const String &p_name) const {
	ERR_FAIL_COND_V(!data.has(p_name), Variant());
	return data.get(p_name).get_value();
}

void BlackboardPlan::add_var(const String &p_name, const BBVariable &p_var) {
	ERR_FAIL_COND(data.has(p_name));
	ERR_FAIL_COND(base.is_valid());
	data.insert(p_name, p_var);
	notify_property_list_changed();
	emit_changed();
}

void BlackboardPlan::remove_var(const String &p_name) {
	ERR_FAIL_COND(!data.has(p_name));
	ERR_FAIL_COND(base.is_valid());
	data.erase(p_name);
	notify_property_list_changed();
	emit_changed();
}

BBVariable BlackboardPlan::get_var(const String &p_name) {
	ERR_FAIL_COND_V(!data.has(p_name), BBVariable());
	return data.get(p_name);
}

Pair<String, BBVariable> BlackboardPlan::get_var_by_index(int p_index) {
	Pair<String, BBVariable> ret;
	ERR_FAIL_INDEX_V(p_index, (int)data.size(), ret);

	int i = 0;
	for (const KeyValue<String, BBVariable> &kv : data) {
		if (i == p_index) {
			ret.first = kv.key;
			ret.second = kv.value;
		}
		i += 1;
	}

	return ret;
}

PackedStringArray BlackboardPlan::list_vars() const {
	PackedStringArray ret;
	for (const KeyValue<String, BBVariable> &kv : data) {
		ret.append(kv.key);
	}
	return ret;
}

String BlackboardPlan::get_var_name(const BBVariable &p_var) const {
	for (const KeyValue<String, BBVariable> &kv : data) {
		if (kv.value == p_var) {
			return kv.key;
		}
	}
	return String();
}

void BlackboardPlan::rename_var(const String &p_name, const String &p_new_name) {
	ERR_FAIL_COND(p_new_name.is_empty());
	ERR_FAIL_COND(data.has(p_new_name));
	ERR_FAIL_COND(!data.has(p_name));

	data.replace_key(p_name, p_new_name);
	notify_property_list_changed();
	emit_changed();
}

void BlackboardPlan::swap_vars(int p_idx_a, int p_idx_b) {
	ERR_FAIL_INDEX(p_idx_a, (int)data.size());
	ERR_FAIL_INDEX(p_idx_b, (int)data.size());

	Pair<String, BBVariable> a = get_var_by_index(p_idx_a);
	Pair<String, BBVariable> b = get_var_by_index(p_idx_b);

	data.replace_key(a.first, "__tmp__");
	data.replace_key(b.first, a.first);
	data.replace_key("__tmp__", b.first);

	data[b.first] = b.second;
	data[a.first] = a.second;

	notify_property_list_changed();
	emit_changed();
}

void BlackboardPlan::sync_with_base_plan() {
	if (base.is_null()) {
		return;
	}

	bool changed = false;

	// Sync variables with the base plan.
	for (const KeyValue<String, BBVariable> &kv : base->data) {
		if (!data.has(kv.key)) {
			data.insert(kv.key, kv.value.duplicate());
			changed = true;
			continue;
		}

		BBVariable var = data.get(kv.key);
		if (!var.is_same_prop_info(kv.value)) {
			var.copy_prop_info(kv.value);
			changed = true;
		}
		if (var.get_value().get_type() != kv.value.get_type()) {
			var.set_value(kv.value.get_value());
			changed = true;
		}
	}

	// Erase variables that do not exist in the base plan.
	for (const KeyValue<String, BBVariable> &kv : data) {
		if (!base->data.has(kv.key)) {
			data.erase(kv.key);
			changed = true;
		}
	}

	if (changed) {
		notify_property_list_changed();
		emit_changed();
	}
}

Ref<Blackboard> BlackboardPlan::create_blackboard() {
	Ref<Blackboard> bb = memnew(Blackboard);
	for (const KeyValue<String, BBVariable> &kv : data) {
		bb->add_var(kv.key, kv.value.duplicate());
	}
	return bb;
}

void BlackboardPlan::populate_blackboard(const Ref<Blackboard> &p_blackboard, bool overwrite) {
	for (const KeyValue<String, BBVariable> &kv : data) {
		if (p_blackboard->has_var(kv.key)) {
			if (overwrite) {
				p_blackboard->erase_var(kv.key);
			} else {
				continue;
			}
		}
		p_blackboard->add_var(kv.key, kv.value.duplicate());
	}
}

BlackboardPlan::BlackboardPlan() {
}
