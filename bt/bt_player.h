/* bt_player.h */

#ifndef BT_PLAYER_H
#define BT_PLAYER_H

#include "behavior_tree.h"
#include "bt_task.h"
#include "core/object/object.h"
#include "modules/limboai/blackboard.h"
#include "scene/main/node.h"

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
	Ref<Blackboard> blackboard;
	bool prefetch_nodepath_vars = true;

	Ref<BehaviorTree> _loaded_tree;
	Ref<BTTask> _root_task;

	void _load_tree();

protected:
	static void _bind_methods();

	void _set_blackboard_data(Dictionary p_value) { blackboard->set_data(p_value.duplicate()); }
	Dictionary _get_blackboard_data() const { return blackboard->get_data(); }

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

	Ref<Blackboard> get_blackboard() const { return blackboard; }
	void set_blackboard(const Ref<Blackboard> &p_blackboard) { blackboard = p_blackboard; }

	void set_prefetch_nodepath_vars(bool p_value) { prefetch_nodepath_vars = p_value; }
	bool get_prefetch_nodepath_vars() const { return prefetch_nodepath_vars; }

	void update(float p_delta);
	void restart();

	BTPlayer();
	~BTPlayer();
};

#endif // BT_PLAYER_H