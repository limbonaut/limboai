/* behavior_tree.cpp */

#include "behavior_tree.h"
#include "core/class_db.h"
#include "core/list.h"
#include "core/object.h"
#include "core/variant.h"
#include <cstddef>

void BehaviorTree::init() {
	List<BTTask *> stack;
	BTTask *task = root_task.ptr();
	while (task != nullptr) {
		for (int i = 0; i < task->get_child_count(); i++) {
			task->get_child(i)->parent = task;
			stack.push_back(task->get_child(i).ptr());
		}
		task = nullptr;
		if (!stack.empty()) {
			task = stack.front()->get();
			stack.pop_front();
		}
	}
}

Ref<BehaviorTree> BehaviorTree::clone() const {
	Ref<BehaviorTree> copy = duplicate(false);
	copy->set_path("");
	if (root_task.is_valid()) {
		copy->root_task = root_task->clone();
	}
	return copy;
}

void BehaviorTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_description", "p_value"), &BehaviorTree::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &BehaviorTree::get_description);
	ClassDB::bind_method(D_METHOD("set_root_task", "p_value"), &BehaviorTree::set_root_task);
	ClassDB::bind_method(D_METHOD("get_root_task"), &BehaviorTree::get_root_task);
	ClassDB::bind_method(D_METHOD("init"), &BehaviorTree::init);
	ClassDB::bind_method(D_METHOD("clone"), &BehaviorTree::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description", PROPERTY_HINT_MULTILINE_TEXT), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root_task", PROPERTY_HINT_RESOURCE_TYPE, "BTTask"), "set_root_task", "get_root_task");
}