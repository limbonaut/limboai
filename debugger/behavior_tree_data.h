/* behavior_tree_data.h */

#ifndef BEHAVIOR_TREE_DATA_H
#define BEHAVIOR_TREE_DATA_H

#include "core/object/object.h"
#include "modules/limboai/bt/bt_task.h"

class BehaviorTreeData {
public:
	struct TaskData {
		String name;
		// ObjectID object_id;
		int num_children = 0;
		int status = 0;
		int last_tick_usec = 0;
		String type_name;
		// String script_path;
		// String resource_path;

		TaskData(const String &p_name, int p_num_children, int p_status, int p_last_tick_usec, const String &p_type_name) {
			name = p_name;
			num_children = p_num_children;
			status = p_status;
			last_tick_usec = p_last_tick_usec;
			type_name = p_type_name;
		}

		TaskData() {}
	};

	List<TaskData> tasks;

	void serialize(Array &p_arr);
	void deserialize(const Array &p_arr);

	BehaviorTreeData(const Ref<BTTask> &p_instance);
	BehaviorTreeData() {}
};

#endif // BEHAVIOR_TREE_DATA