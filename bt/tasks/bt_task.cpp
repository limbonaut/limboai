/**
 * bt_task.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_task.h"

#include "bt_comment.h"
#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/util/limbo_string_names.h"
#include "modules/limboai/util/limbo_utility.h"

#include "core/error/error_macros.h"
#include "core/io/resource.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/object/script_language.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/variant/variant.h"

void BT::_bind_methods() {
	BIND_ENUM_CONSTANT(FRESH);
	BIND_ENUM_CONSTANT(RUNNING);
	BIND_ENUM_CONSTANT(FAILURE);
	BIND_ENUM_CONSTANT(SUCCESS);
}

String BTTask::_generate_name() const {
	if (get_script_instance()) {
		if (get_script_instance()->has_method(LimboStringNames::get_singleton()->_generate_name)) {
			ERR_FAIL_COND_V_MSG(!get_script_instance()->get_script()->is_tool(), "ERROR: not a tool script", "Task script should be a \"tool\" script!");
			return get_script_instance()->call(LimboStringNames::get_singleton()->_generate_name);
		}
		String script_path = get_script_instance()->get_script()->get_path();
		if (!script_path.is_empty()) {
			// Generate name based on script file
			return script_path.get_basename().get_file().trim_prefix("BT").to_pascal_case();
		}
	}
	return get_class().trim_prefix("BT");
}

Array BTTask::_get_children() const {
	Array arr;
	int num_children = get_child_count();
	arr.resize(num_children);
	for (int i = 0; i < num_children; i++) {
		arr[i] = get_child(i).ptr();
	}

	return arr;
}

void BTTask::_set_children(Array p_children) {
	data.children.clear();
	const int num_children = p_children.size();
	data.children.resize(num_children);
	for (int i = 0; i < num_children; i++) {
		Variant task_var = p_children[i];
		Ref<BTTask> task_ref = task_var;
		task_ref->data.parent = this;
		data.children.set(i, task_var);
	}
}

String BTTask::get_task_name() const {
	if (data.custom_name.is_empty()) {
		return _generate_name();
	}
	return data.custom_name;
}

Ref<BTTask> BTTask::get_root() const {
	const BTTask *task = this;
	while (!task->is_root()) {
		task = task->data.parent;
	}
	return Ref<BTTask>(task);
}

void BTTask::set_custom_name(const String &p_name) {
	if (data.custom_name != p_name) {
		data.custom_name = p_name;
		emit_changed();
	}
};

void BTTask::initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard) {
	ERR_FAIL_COND(p_agent == nullptr);
	ERR_FAIL_COND(p_blackboard == nullptr);
	data.agent = p_agent;
	data.blackboard = p_blackboard;
	for (int i = 0; i < data.children.size(); i++) {
		get_child(i)->initialize(p_agent, p_blackboard);
	}

	if (!GDVIRTUAL_CALL(_setup)) {
		_setup();
	}
}

Ref<BTTask> BTTask::clone() const {
	Ref<BTTask> inst = duplicate(false);
	inst->data.parent = nullptr;
	inst->data.agent = nullptr;
	inst->data.blackboard.unref();
	int num_null = 0;
	for (int i = 0; i < data.children.size(); i++) {
		Ref<BTTask> c = get_child(i)->clone();
		if (c.is_valid()) {
			c->data.parent = inst.ptr();
			inst->data.children.set(i - num_null, c);
		} else {
			num_null += 1;
		}
	}
	if (num_null > 0) {
		// * BTComment tasks return nullptr at runtime - we remove those.
		inst->data.children.resize(data.children.size() - num_null);
	}

	// Make BBParam properties unique.
	List<PropertyInfo> props;
	inst->get_property_list(&props);
	HashMap<Ref<Resource>, Ref<Resource>> duplicates;
	for (List<PropertyInfo>::Element *E = props.front(); E; E = E->next()) {
		if (!(E->get().usage & PROPERTY_USAGE_STORAGE)) {
			continue;
		}

		Variant v = inst->get(E->get().name);

		if (v.is_ref_counted()) {
			Ref<RefCounted> ref = v;
			if (ref.is_valid()) {
				Ref<Resource> res = ref;
				if (res.is_valid() && res->is_class("BBParam")) {
					if (!duplicates.has(res)) {
						duplicates[res] = res->duplicate();
					}
					res = duplicates[res];
					inst->set(E->get().name, res);
				}
			}
		}
	}

	return inst;
}

BT::Status BTTask::execute(double p_delta) {
	if (data.status != RUNNING) {
		// Reset children status.
		if (data.status != FRESH) {
			for (int i = 0; i < get_child_count(); i++) {
				data.children.get(i)->cancel();
			}
		}
		if (!GDVIRTUAL_CALL(_enter)) {
			_enter();
		}
	} else {
		data.elapsed += p_delta;
	}

	if (!GDVIRTUAL_CALL(_tick, p_delta, data.status)) {
		data.status = _tick(p_delta);
	}

	if (data.status != RUNNING) {
		if (!GDVIRTUAL_CALL(_exit)) {
			_exit();
		}
		data.elapsed = 0.0;
	}
	return data.status;
}

void BTTask::cancel() {
	for (int i = 0; i < data.children.size(); i++) {
		get_child(i)->cancel();
	}
	if (data.status == RUNNING) {
		if (!GDVIRTUAL_CALL(_exit)) {
			_exit();
		}
	}
	data.status = FRESH;
	data.elapsed = 0.0;
}

Ref<BTTask> BTTask::get_child(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, data.children.size(), nullptr);
	return data.children.get(p_idx);
}

int BTTask::get_child_count() const {
	return data.children.size();
}

int BTTask::get_child_count_excluding_comments() const {
	int count = 0;
	for (int i = 0; i < data.children.size(); i++) {
		if (!data.children[i]->is_class_ptr(BTComment::get_class_ptr_static())) {
			count += 1;
		}
	}
	return count;
}

void BTTask::add_child(Ref<BTTask> p_child) {
	ERR_FAIL_COND_MSG(p_child->get_parent().is_valid(), "p_child already has a parent!");
	p_child->data.parent = this;
	data.children.push_back(p_child);
	emit_changed();
}

void BTTask::add_child_at_index(Ref<BTTask> p_child, int p_idx) {
	ERR_FAIL_COND_MSG(p_child->get_parent().is_valid(), "p_child already has a parent!");
	if (p_idx < 0 || p_idx > data.children.size()) {
		p_idx = data.children.size();
	}
	data.children.insert(p_idx, p_child);
	p_child->data.parent = this;
	emit_changed();
}

void BTTask::remove_child(Ref<BTTask> p_child) {
	int idx = data.children.find(p_child);
	if (idx == -1) {
		ERR_FAIL_MSG("p_child not found!");
	} else {
		data.children.remove_at(idx);
		p_child->data.parent = nullptr;
		emit_changed();
	}
}

void BTTask::remove_child_at_index(int p_idx) {
	ERR_FAIL_INDEX(p_idx, get_child_count());
	data.children.remove_at(p_idx);
}

bool BTTask::has_child(const Ref<BTTask> &p_child) const {
	return data.children.find(p_child) != -1;
}

bool BTTask::is_descendant_of(const Ref<BTTask> &p_task) const {
	const BTTask *task = this;
	while (task != nullptr) {
		task = task->data.parent;
		if (task == p_task.ptr()) {
			return true;
		}
	}
	return false;
}

int BTTask::get_child_index(const Ref<BTTask> &p_child) const {
	return data.children.find(p_child);
}

Ref<BTTask> BTTask::next_sibling() const {
	if (data.parent != nullptr) {
		int idx = data.parent->get_child_index(Ref<BTTask>(this));
		if (idx != -1 && data.parent->get_child_count() > (idx + 1)) {
			return data.parent->get_child(idx + 1);
		}
	}
	return Ref<BTTask>();
}

PackedStringArray BTTask::get_configuration_warnings() const {
	PackedStringArray ret;

	PackedStringArray warnings;
	if (GDVIRTUAL_CALL(_get_configuration_warning, warnings)) {
		ret.append_array(warnings);
	}

	return ret;
}

void BTTask::print_tree(int p_initial_tabs) const {
	String tabs = "--";
	for (int i = 0; i < p_initial_tabs; i++) {
		tabs += "--";
	}
	print_line(vformat("%s Name: %s Instance: %s", tabs, get_task_name(), Ref<BTTask>(this)));
	for (int i = 0; i < get_child_count(); i++) {
		get_child(i)->print_tree(p_initial_tabs + 1);
	}
}

void BTTask::_bind_methods() {
	// Public Methods.
	ClassDB::bind_method(D_METHOD("is_root"), &BTTask::is_root);
	ClassDB::bind_method(D_METHOD("get_root"), &BTTask::get_root);
	ClassDB::bind_method(D_METHOD("initialize", "p_agent", "p_blackboard"), &BTTask::initialize);
	ClassDB::bind_method(D_METHOD("clone"), &BTTask::clone);
	ClassDB::bind_method(D_METHOD("execute", "p_delta"), &BTTask::execute);
	ClassDB::bind_method(D_METHOD("get_child", "p_idx"), &BTTask::get_child);
	ClassDB::bind_method(D_METHOD("get_child_count"), &BTTask::get_child_count);
	ClassDB::bind_method(D_METHOD("get_child_count_excluding_comments"), &BTTask::get_child_count_excluding_comments);
	ClassDB::bind_method(D_METHOD("add_child", "p_child"), &BTTask::add_child);
	ClassDB::bind_method(D_METHOD("add_child_at_index", "p_child", "p_idx"), &BTTask::add_child_at_index);
	ClassDB::bind_method(D_METHOD("remove_child", "p_child"), &BTTask::remove_child);
	ClassDB::bind_method(D_METHOD("remove_child_at_index", "p_idx"), &BTTask::remove_child_at_index);
	ClassDB::bind_method(D_METHOD("has_child", "p_child"), &BTTask::has_child);
	ClassDB::bind_method(D_METHOD("is_descendant_of", "p_task"), &BTTask::is_descendant_of);
	ClassDB::bind_method(D_METHOD("get_child_index", "p_child"), &BTTask::get_child_index);
	ClassDB::bind_method(D_METHOD("next_sibling"), &BTTask::next_sibling);
	ClassDB::bind_method(D_METHOD("print_tree", "p_initial_tabs"), &BTTask::print_tree, Variant(0));
	ClassDB::bind_method(D_METHOD("get_task_name"), &BTTask::get_task_name);
	ClassDB::bind_method(D_METHOD("get_custom_name"), &BTTask::get_custom_name);
	ClassDB::bind_method(D_METHOD("set_custom_name", "p_name"), &BTTask::set_custom_name);

	// Properties, setters and getters.
	ClassDB::bind_method(D_METHOD("get_agent"), &BTTask::get_agent);
	ClassDB::bind_method(D_METHOD("set_agent", "p_agent"), &BTTask::set_agent);
	ClassDB::bind_method(D_METHOD("_get_children"), &BTTask::_get_children);
	ClassDB::bind_method(D_METHOD("_set_children", "p_children"), &BTTask::_set_children);
	ClassDB::bind_method(D_METHOD("get_blackboard"), &BTTask::get_blackboard);
	ClassDB::bind_method(D_METHOD("get_parent"), &BTTask::get_parent);
	ClassDB::bind_method(D_METHOD("get_status"), &BTTask::get_status);
	ClassDB::bind_method(D_METHOD("get_elapsed_time"), &BTTask::get_elapsed_time);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "custom_name"), "set_custom_name", "get_custom_name");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "agent", PROPERTY_HINT_RESOURCE_TYPE, "Node", PROPERTY_USAGE_NONE), "set_agent", "get_agent");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "blackboard", PROPERTY_HINT_RESOURCE_TYPE, "Blackboard", PROPERTY_USAGE_NONE), "", "get_blackboard");
	// ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "parent", PROPERTY_HINT_RESOURCE_TYPE, "BTTask", 0), "", "get_parent");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL), "_set_children", "_get_children");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "status", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "", "get_status");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "elapsed_time", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "", "get_elapsed_time");

	GDVIRTUAL_BIND(_setup);
	GDVIRTUAL_BIND(_enter);
	GDVIRTUAL_BIND(_exit);
	GDVIRTUAL_BIND(_tick, "p_delta");
	GDVIRTUAL_BIND(_generate_name);
	GDVIRTUAL_BIND(_get_configuration_warning);
}

BTTask::BTTask() {
	data.custom_name = String();
	data.agent = nullptr;
	data.parent = nullptr;
	data.children = Vector<Ref<BTTask>>();
	data.status = FRESH;
	data.elapsed = 0.0;
}

BTTask::~BTTask() {
	for (int i = 0; i < get_child_count(); i++) {
		ERR_FAIL_COND(!get_child(i).is_valid());
		get_child(i)->data.parent = nullptr;
		get_child(i).unref();
	}
}
