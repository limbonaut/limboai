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
	ERR_FAIL_COND(!data.has(p_name));
	data.get(p_name).set_value(p_value);
}

Variant BlackboardSource::get_value(const String &p_name) const {
	ERR_FAIL_COND_V(!data.has(p_name), Variant());
	return data.get(p_name).get_value();
}

void BlackboardSource::add_var(const String &p_name, const BBVariable &p_var) {
	ERR_FAIL_COND(data.has(p_name));
	ERR_FAIL_COND(base.is_valid());
	data.insert(p_name, p_var);
}

void BlackboardSource::remove_var(const String &p_name) {
	ERR_FAIL_COND(!data.has(p_name));
	ERR_FAIL_COND(base.is_valid());
	data.erase(p_name);
}

BBVariable BlackboardSource::get_var(const String &p_name) {
	ERR_FAIL_COND_V(!data.has(p_name), BBVariable());
	return data.get(p_name);
}

PackedStringArray BlackboardSource::list_vars() const {
	PackedStringArray ret;
	for (const KeyValue<String, BBVariable> &kv : data) {
		ret.append(kv.key);
	}
	return ret;
}

void BlackboardSource::sync_base() {
	for (const KeyValue<String, BBVariable> &kv : base->data) {
		if (!data.has(kv.key)) {
			data.insert(kv.key, kv.value.duplicate());
			continue;
		}

		BBVariable var = data.get(kv.key);
		if (!var.is_same_prop_info(kv.value)) {
			var.copy_prop_info(kv.value);
		}
		if (var.get_value().get_type() != kv.value.get_type()) {
			var.set_value(kv.value.get_value());
		}
	}
}

Ref<Blackboard> BlackboardSource::create_blackboard() {
	Ref<Blackboard> bb = memnew(Blackboard);
	for (const KeyValue<String, BBVariable> &kv : data) {
		bb->add_var(kv.key, kv.value.duplicate());
	}
	return bb;
}

void BlackboardSource::populate_blackboard(const Ref<Blackboard> &p_blackboard, bool overwrite) {
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
