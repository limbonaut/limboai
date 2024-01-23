/**
 * blackboard_source.cpp
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "blackboard_source.h"

void BlackboardSource::set_value(const String &p_name, const Variant &p_value) {
	ERR_FAIL_COND(!vars.has(p_name));
	vars.get(p_name).set_value(p_value);
}

Variant BlackboardSource::get_value(const String &p_name) const {
	ERR_FAIL_COND_V(!vars.has(p_name), Variant());
	return vars.get(p_name).get_value();
}

void BlackboardSource::add_var(const String &p_name, const BBVariable &p_var) {
	ERR_FAIL_COND(vars.has(p_name));
	ERR_FAIL_COND(base.is_valid());
	vars.insert(p_name, p_var);
}

void BlackboardSource::remove_var(const String &p_name) {
	ERR_FAIL_COND(!vars.has(p_name));
	ERR_FAIL_COND(base.is_valid());
	vars.erase(p_name);
}

BBVariable BlackboardSource::get_var(const String &p_name) {
	ERR_FAIL_COND_V(!vars.has(p_name), BBVariable());
	return vars.get(p_name);
}

PackedStringArray BlackboardSource::list_vars() const {
	PackedStringArray ret;
	for (const KeyValue<String, BBVariable> &kv : vars) {
		ret.append(kv.key);
	}
	return ret;
}

void BlackboardSource::sync_base() {
	for (const KeyValue<String, BBVariable> &kv : base->vars) {
		if (!vars.has(kv.key)) {
			vars.insert(kv.key, kv.value.duplicate());
			continue;
		}

		BBVariable var = vars.get(kv.key);
		if (!var.is_same_prop_info(kv.value)) {
			var.copy_prop_info(kv.value);
		}
		if (var.get_value().get_type() != kv.value.get_type()) {
			var.set_value(kv.value.get_value());
		}
	}
}

Ref<Blackboard> BlackboardSource::instantiate() {
	Ref<Blackboard> bb = memnew(Blackboard);
	// TODO: fill bb
	return bb;
}
