/* bt_state.h */

#ifndef BT_STATE_H
#define BT_STATE_H

#include "core/object.h"
#include "modules/limboai/bt/behavior_tree.h"
#include "modules/limboai/bt/bt_task.h"
#include "modules/limboai/limbo_state.h"

class BTState : public LimboState {
	GDCLASS(BTState, LimboState);

private:
	Ref<BehaviorTree> behavior_tree;
	Ref<Blackboard> blackboard;
	Ref<BTTask> root_task;

protected:
	static void _bind_methods();

	virtual void _setup();
	// virtual void _enter() {}
	virtual void _exit();
	virtual void _update(float p_delta);

public:
	void set_behavior_tree(const Ref<BehaviorTree> &p_value) { behavior_tree = p_value; }
	Ref<BehaviorTree> get_behavior_tree() const { return behavior_tree; }
	// void set_blackboard(const Ref<Blackboard> &p_value) { blackboard = p_value; }
	Ref<Blackboard> get_blackboard() const { return blackboard; }
};

#endif // BT_STATE_H