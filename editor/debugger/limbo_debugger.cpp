/**
 * limbo_debugger.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "limbo_debugger.h"

#include "behavior_tree_data.h"
#include "modules/limboai/bt/tasks/bt_task.h"

#include "core/debugger/engine_debugger.h"
#include "core/error/error_macros.h"
#include "core/io/resource.h"
#include "core/string/node_path.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"

//// LimboDebugger

LimboDebugger *LimboDebugger::singleton = nullptr;
LimboDebugger *LimboDebugger::get_singleton() {
	return singleton;
}

LimboDebugger::LimboDebugger() {
	singleton = this;
#ifdef DEBUG_ENABLED
	EngineDebugger::register_message_capture("limboai", EngineDebugger::Capture(nullptr, LimboDebugger::parse_message));
#endif
}

LimboDebugger::~LimboDebugger() {
	singleton = nullptr;
}

void LimboDebugger::initialize() {
	if (EngineDebugger::is_active()) {
		memnew(LimboDebugger);
	}
}

void LimboDebugger::deinitialize() {
	if (singleton) {
		memdelete(singleton);
	}
}

#ifdef DEBUG_ENABLED
Error LimboDebugger::parse_message(void *p_user, const String &p_msg, const Array &p_args, bool &r_captured) {
	r_captured = true;
	if (p_msg == "track_bt_player") {
		singleton->_track_tree(p_args[0]);
	} else if (p_msg == "untrack_bt_player") {
		singleton->_untrack_tree();
	} else if (p_msg == "start_session") {
		singleton->session_active = true;
		singleton->_send_active_bt_players();
	} else if (p_msg == "stop_session") {
		singleton->session_active = false;
	} else {
		r_captured = false;
	}
	return OK;
}

void LimboDebugger::register_bt_instance(Ref<BTTask> p_instance, NodePath p_player_path) {
	ERR_FAIL_COND(p_instance.is_null());
	ERR_FAIL_COND(p_player_path.is_empty());
	if (active_trees.has(p_player_path)) {
		return;
	}

	active_trees.insert(p_player_path, p_instance);
	if (session_active) {
		_send_active_bt_players();
	}
}

void LimboDebugger::unregister_bt_instance(Ref<BTTask> p_instance, NodePath p_player_path) {
	ERR_FAIL_COND(p_instance.is_null());
	ERR_FAIL_COND(p_player_path.is_empty());
	ERR_FAIL_COND(!active_trees.has(p_player_path));

	if (tracked_player == p_player_path) {
		_untrack_tree();
	}
	active_trees.erase(p_player_path);

	if (session_active) {
		_send_active_bt_players();
	}
}

void LimboDebugger::_track_tree(NodePath p_path) {
	ERR_FAIL_COND(!active_trees.has(p_path));

	if (!tracked_player.is_empty()) {
		_untrack_tree();
	}

	Node *node = SceneTree::get_singleton()->get_root()->get_node(p_path);
	ERR_FAIL_COND(node == nullptr);

	tracked_player = p_path;

	bool r_valid = false;
	Ref<Resource> bt = node->get(SNAME("behavior_tree"), &r_valid);
	if (bt.is_valid()) {
		bt_resource_path = bt->get_path();
	} else {
		bt_resource_path = "";
	}

	if (node->is_class("BTPlayer")) {
		node->connect(SNAME("updated"), callable_mp(this, &LimboDebugger::_on_bt_updated).bind(p_path));
	} else if (node->is_class("BTState")) {
		node->connect(SNAME("updated"), callable_mp(this, &LimboDebugger::_on_state_updated).bind(p_path));
	}
}

void LimboDebugger::_untrack_tree() {
	if (tracked_player.is_empty()) {
		return;
	}

	NodePath was_tracking = tracked_player;
	tracked_player = NodePath();

	Node *node = SceneTree::get_singleton()->get_root()->get_node(was_tracking);
	ERR_FAIL_COND(node == nullptr);

	if (node->is_class("BTPlayer")) {
		node->disconnect(SNAME("updated"), callable_mp(this, &LimboDebugger::_on_bt_updated));
	} else if (node->is_class("BTState")) {
		node->disconnect(SNAME("updated"), callable_mp(this, &LimboDebugger::_on_state_updated));
	}
}

void LimboDebugger::_send_active_bt_players() {
	Array arr;
	for (KeyValue<NodePath, Ref<BTTask>> kv : active_trees) {
		arr.append(kv.key);
	}
	EngineDebugger::get_singleton()->send_message("limboai:active_bt_players", arr);
}

void LimboDebugger::_on_bt_updated(int _status, NodePath p_path) {
	if (p_path != tracked_player) {
		return;
	}
	Array arr;
	BehaviorTreeData(active_trees.get(tracked_player), tracked_player, bt_resource_path).serialize(arr);
	EngineDebugger::get_singleton()->send_message("limboai:bt_update", arr);
}

void LimboDebugger::_on_state_updated(float _delta, NodePath p_path) {
	if (p_path != tracked_player) {
		return;
	}
	Array arr;
	BehaviorTreeData(active_trees.get(tracked_player), tracked_player, bt_resource_path).serialize(arr);
	EngineDebugger::get_singleton()->send_message("limboai:bt_update", arr);
}

#endif // DEBUG_ENABLED
