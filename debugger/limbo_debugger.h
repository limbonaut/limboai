/* limbo_debugger.h */

#ifndef LIMBO_DEBUGGER_H
#define LIMBO_DEBUGGER_H

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/string/node_path.h"
#include "modules/limboai/bt/bt_task.h"

class LimboDebugger : public Object {
	GDCLASS(LimboDebugger, Object);

private:
	static LimboDebugger *singleton;

	LimboDebugger();

public:
	static void initialize();
	static void deinitialize();
	static LimboDebugger *get_singleton();

	~LimboDebugger();

#ifdef DEBUG_ENABLED
private:
	HashMap<NodePath, Ref<BTTask>> active_trees;
	NodePath tracked_tree;
	bool session_active = false;

	void _track_tree(NodePath p_path);
	void _untrack_tree();
	void _send_active_bt_players();

	void _on_bt_updated(int status, NodePath p_path);
	void _on_state_updated(float _delta, NodePath p_path);

public:
	static Error parse_message(void *p_user, const String &p_msg, const Array &p_args, bool &r_captured);

	void register_bt_instance(Ref<BTTask> p_instance, NodePath p_player_path);
	void unregister_bt_instance(Ref<BTTask> p_instance, NodePath p_player_path);

#endif // DEBUG_ENABLED
};
#endif // LIMBO_DEBUGGER