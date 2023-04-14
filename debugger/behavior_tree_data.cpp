/* behavior_tree_data.cpp */

#include "behavior_tree_data.h"
#include "core/templates/list.h"

//// BehaviorTreeData

BehaviorTreeData::BehaviorTreeData(const Ref<BTTask> &p_instance) {
	// Flatten tree into list depth first
	List<Ref<BTTask>> stack;
	stack.push_back(p_instance);
	int id = 0;
	while (stack.size()) {
		Ref<BTTask> task = stack[0];
		stack.pop_front();

		int num_children = task->get_child_count();
		for (int i = 0; i < num_children; i++) {
			stack.push_front(task->get_child(num_children - 1 - i));
		}

		tasks.push_back(TaskData(
				id,
				task->get_task_name(),
				num_children,
				task->get_status(),
				task->get_elapsed_time(),
				task->get_class()));
		id += 1;
	}
}

void BehaviorTreeData::serialize(Array &p_arr) {
	for (const TaskData &td : tasks) {
		p_arr.push_back(td.id);
		p_arr.push_back(td.name);
		p_arr.push_back(td.num_children);
		p_arr.push_back(td.status);
		p_arr.push_back(td.elapsed_time);
		p_arr.push_back(td.type_name);
	}
}

void BehaviorTreeData::deserialize(const Array &p_arr) {
	ERR_FAIL_COND(tasks.size() != 0);

	int idx = 0;
	while (p_arr.size() > idx) {
		ERR_FAIL_COND(p_arr.size() < 6);
		ERR_FAIL_COND(p_arr[idx].get_type() != Variant::INT);
		ERR_FAIL_COND(p_arr[idx + 1].get_type() != Variant::STRING);
		ERR_FAIL_COND(p_arr[idx + 2].get_type() != Variant::INT);
		ERR_FAIL_COND(p_arr[idx + 3].get_type() != Variant::INT);
		ERR_FAIL_COND(p_arr[idx + 4].get_type() != Variant::FLOAT);
		ERR_FAIL_COND(p_arr[idx + 5].get_type() != Variant::STRING);
		tasks.push_back(TaskData(p_arr[idx], p_arr[idx + 1], p_arr[idx + 2], p_arr[idx + 3], p_arr[idx + 4], p_arr[idx + 5]));
		idx += 6;
	}
}
