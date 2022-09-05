/* bt_player.h */

#ifndef BT_PLAYER_H
#define BT_PLAYER_H

#include "core/object.h"
#include "scene/main/node.h"

#include "behavior_tree.h"
#include "bt_task.h"
#include <pulse/proplist.h>

class BTPlayer : public Node {
	GDCLASS(BTPlayer, Node);

public:
	enum UpdateMode : unsigned int {
		IDLE, // automatically call update() during NOTIFICATION_PROCESS
		PHYSICS, //# automatically call update() during NOTIFICATION_PHYSICS
		MANUAL, // manually update state machine, user must call update(delta)
	};

private:
	Ref<BehaviorTree> behavior_tree;
	UpdateMode update_mode = UpdateMode::IDLE;
	bool active = false;
	bool auto_restart = false;
	Dictionary blackboard;

	Ref<BehaviorTree> _loaded_tree;
	Ref<BTTask> _root_task;

	void _load_tree();

protected:
	static void _bind_methods();

	void _notification(int p_notification);

public:
	void set_behavior_tree(const Ref<BehaviorTree> &p_tree);
	Ref<BehaviorTree> get_behavior_tree() const { return behavior_tree; };

	void set_update_mode(UpdateMode p_mode);
	UpdateMode get_update_mode() const { return update_mode; }

	void set_active(bool p_active);
	bool get_active() const { return active; }

	void set_auto_restart(bool p_value) { auto_restart = p_value; }
	bool get_auto_restart() const { return auto_restart; }

	void set_blackboard(Dictionary p_value) { blackboard = p_value; }
	Dictionary get_blackboard() const { return blackboard; }

	void update(float p_delta);
	void restart();
};

#endif // BT_PLAYER_H