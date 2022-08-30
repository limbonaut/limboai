/* bt_player.cpp */

#include "bt_player.h"

#include "../limbo_string_names.h"
#include "bt_task.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "core/io/resource_loader.h"
#include "core/object.h"
#include <cstddef>

VARIANT_ENUM_CAST(BTPlayer::UpdateMode);

void BTPlayer::_load_tree() {
	_loaded_tree.unref();
	_root_task.unref();
	ERR_FAIL_COND_MSG(!behavior_tree.is_valid(), "BTPlayer needs a valid behavior tree.");
	ERR_FAIL_COND_MSG(!behavior_tree->get_root_task().is_valid(), "Behavior tree has no valid root task.");
	_loaded_tree = behavior_tree;
	_root_task = _loaded_tree->get_root_task()->clone();
	_root_task->initialize(get_owner(), blackboard);
}

void BTPlayer::set_behavior_tree(const Ref<BehaviorTree> &p_tree) {
	behavior_tree = p_tree;
	if (Engine::get_singleton()->is_editor_hint() == false) {
		_load_tree();
		set_update_mode(update_mode);
	}
}

void BTPlayer::set_update_mode(UpdateMode p_mode) {
	update_mode = p_mode;
	set_active(active);
}

void BTPlayer::set_active(bool p_active) {
	active = p_active;
	if (!Engine::get_singleton()->is_editor_hint()) {
		set_process(update_mode == UpdateMode::IDLE);
		set_physics_process(update_mode == UpdateMode::PHYSICS);
	}
}

void BTPlayer::update(float p_delta) {
	if (!_root_task.is_valid()) {
		ERR_PRINT_ONCE(vformat("BTPlayer has no root task to update (owner: %s)", get_owner()));
		return;
	}
	if (active) {
		int status = _root_task->execute(p_delta);
		if (status == BTTask::SUCCESS || status == BTTask::FAILURE) {
			set_active(auto_restart);
			emit_signal(LimboStringNames::get_singleton()->behavior_tree_finished, status);
		}
	}
}

void BTPlayer::restart() {
	_root_task->cancel();
	set_active(true);
}

void BTPlayer::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			if (active) {
				Variant time = get_process_delta_time();
				update(time);
			}
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			if (active) {
				Variant time = get_process_delta_time();
				update(time);
			}
		} break;
		case NOTIFICATION_READY: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				if (behavior_tree.is_valid()) {
					_load_tree();
				}
				set_active(active);
			}
		} break;
	}
}

void BTPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_behavior_tree", "p_path"), &BTPlayer::set_behavior_tree);
	ClassDB::bind_method(D_METHOD("get_behavior_tree"), &BTPlayer::get_behavior_tree);
	ClassDB::bind_method(D_METHOD("set_update_mode", "p_mode"), &BTPlayer::set_update_mode);
	ClassDB::bind_method(D_METHOD("get_update_mode"), &BTPlayer::get_update_mode);
	ClassDB::bind_method(D_METHOD("set_active", "p_active"), &BTPlayer::set_active);
	ClassDB::bind_method(D_METHOD("get_active"), &BTPlayer::get_active);
	ClassDB::bind_method(D_METHOD("set_auto_restart", "p_value"), &BTPlayer::set_auto_restart);
	ClassDB::bind_method(D_METHOD("get_auto_restart"), &BTPlayer::get_auto_restart);

	ClassDB::bind_method(D_METHOD("update", "p_delta"), &BTPlayer::update);
	ClassDB::bind_method(D_METHOD("restart"), &BTPlayer::restart);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "behavior_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviorTree"), "set_behavior_tree", "get_behavior_tree");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "update_mode", PROPERTY_HINT_ENUM, "Idle,Physics,Manual"), "set_update_mode", "get_update_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "get_active");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_restart"), "set_auto_restart", "get_auto_restart");

	BIND_ENUM_CONSTANT(IDLE);
	BIND_ENUM_CONSTANT(PHYSICS);
	BIND_ENUM_CONSTANT(MANUAL);
}