/* behavior_tree.h */

#ifndef BEHAVIOR_TREE_H
#define BEHAVIOR_TREE_H

#include "bt_task.h"
#include "core/object.h"
#include "core/resource.h"
#include "modules/limboai/blackboard.h"

class BehaviorTree : public Resource {
	GDCLASS(BehaviorTree, Resource);

private:
	String description;
	Ref<BTTask> root_task;

protected:
	static void _bind_methods();

public:
	void set_description(String p_value) {
		description = p_value;
		emit_changed();
	}
	String get_description() const { return description; }

	void set_root_task(const Ref<BTTask> &p_value) {
		root_task = p_value;
		emit_changed();
	}
	Ref<BTTask> get_root_task() const { return root_task; }

	void init();
	Ref<BehaviorTree> clone() const;
	Ref<BTTask> instance(Object *p_agent, const Ref<Blackboard> &p_blackboard) const;
};

#endif // BEHAVIOR_TREE_H