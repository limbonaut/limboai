/**
 * bt_player.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_player.h"

#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/editor/debugger/limbo_debugger.h"
#include "modules/limboai/util/limbo_string_names.h"

#include "core/config/engine.h"
#include "core/debugger/engine_debugger.h"
#include "core/error/error_macros.h"
#include "core/io/resource_loader.h"
#include "core/object/class_db.h"
#include "core/os/memory.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"
#include "main/performance.h"

VARIANT_ENUM_CAST(BTPlayer::UpdateMode);

void BTPlayer::_load_tree() {
#ifdef DEBUG_ENABLED
	if (tree_instance.is_valid() && EngineDebugger::is_active()) {
		LimboDebugger::get_singleton()->unregister_bt_instance(tree_instance, get_path());
	}
#endif
	tree_instance.unref();
	ERR_FAIL_COND_MSG(!behavior_tree.is_valid(), "BTPlayer: Needs a valid behavior tree.");
	ERR_FAIL_COND_MSG(!behavior_tree->get_root_task().is_valid(), "BTPlayer: Behavior tree has no valid root task.");
	if (prefetch_nodepath_vars == true) {
		blackboard->prefetch_nodepath_vars(this);
	}
	tree_instance = behavior_tree->instantiate(get_owner(), blackboard);
#ifdef DEBUG_ENABLED
	if (EngineDebugger::is_active()) {
		LimboDebugger::get_singleton()->register_bt_instance(tree_instance, get_path());
	}
#endif
}

void BTPlayer::set_behavior_tree(const Ref<BehaviorTree> &p_tree) {
	behavior_tree = p_tree;
	if (Engine::get_singleton()->is_editor_hint() == false && get_owner()) {
		_load_tree();
	}
}

void BTPlayer::set_update_mode(UpdateMode p_mode) {
	update_mode = p_mode;
	set_active(active);
}

void BTPlayer::set_active(bool p_active) {
	active = p_active;
	bool is_not_editor = !Engine::get_singleton()->is_editor_hint();
	set_process(update_mode == UpdateMode::IDLE && active && is_not_editor);
	set_physics_process(update_mode == UpdateMode::PHYSICS && active && is_not_editor);
	set_process_input(active && is_not_editor);
}

void BTPlayer::update(double p_delta) {
	if (!tree_instance.is_valid()) {
		ERR_PRINT_ONCE(vformat("BTPlayer doesn't have a behavior tree with a valid root task to execute (owner: %s)", get_owner()));
		return;
	}

#ifdef DEBUG_ENABLED
	double start = OS::get_singleton()->get_ticks_usec();
#endif

	if (active) {
		last_status = tree_instance->execute(p_delta);
		emit_signal(LimboStringNames::get_singleton()->updated, last_status);
		if (last_status == BTTask::SUCCESS || last_status == BTTask::FAILURE) {
			emit_signal(LimboStringNames::get_singleton()->behavior_tree_finished, last_status);
		}
	}

#ifdef DEBUG_ENABLED
	double end = OS::get_singleton()->get_ticks_usec();
	update_time_acc += (end - start);
	update_time_n += 1.0;
#endif
}

void BTPlayer::restart() {
	tree_instance->abort();
	set_active(true);
}

#ifdef DEBUG_ENABLED

void BTPlayer::_set_monitor_performance(bool p_monitor_performance) {
	monitor_performance = p_monitor_performance;

	if (!get_owner()) {
		return;
	}

	Performance *perf = Performance::get_singleton();
	if (monitor_performance) {
		if (monitor_id == StringName()) {
			monitor_id = vformat("limboai/update_ms|%s_%s_%s", get_owner()->get_name(), get_name(),
					String(itos(get_instance_id())).md5_text().substr(0, 4));
		}
		if (!perf->has_custom_monitor(monitor_id)) {
			perf->add_custom_monitor(monitor_id, callable_mp(this, &BTPlayer::_get_mean_update_time_msec), Vector<Variant>());
		}
	} else if (monitor_id != StringName() && perf->has_custom_monitor(monitor_id)) {
		perf->remove_custom_monitor(monitor_id);
	}
}

double BTPlayer::_get_mean_update_time_msec() {
	if (update_time_n) {
		double mean_time_msec = (update_time_acc * 0.001) / update_time_n;
		update_time_acc = 0.0;
		update_time_n = 0.0;
		return mean_time_msec;
	}
	return 0.0;
}

#endif // DEBUG_ENABLED

void BTPlayer::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			Variant time = get_process_delta_time();
			update(time);
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			Variant time = get_physics_process_delta_time();
			update(time);
		} break;
		case NOTIFICATION_READY: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				if (behavior_tree.is_valid()) {
					_load_tree();
				}
				set_active(active);
#ifdef DEBUG_ENABLED
				_set_monitor_performance(monitor_performance);
#endif
			}
		} break;
#ifdef DEBUG_ENABLED
		case NOTIFICATION_ENTER_TREE: {
			if (tree_instance.is_valid() && EngineDebugger::is_active()) {
				LimboDebugger::get_singleton()->register_bt_instance(tree_instance, get_path());
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (tree_instance.is_valid() && EngineDebugger::is_active()) {
				LimboDebugger::get_singleton()->unregister_bt_instance(tree_instance, get_path());
			}
		} break;
#endif
	}
}

void BTPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_behavior_tree", "p_path"), &BTPlayer::set_behavior_tree);
	ClassDB::bind_method(D_METHOD("get_behavior_tree"), &BTPlayer::get_behavior_tree);
	ClassDB::bind_method(D_METHOD("set_update_mode", "p_mode"), &BTPlayer::set_update_mode);
	ClassDB::bind_method(D_METHOD("get_update_mode"), &BTPlayer::get_update_mode);
	ClassDB::bind_method(D_METHOD("set_active", "p_active"), &BTPlayer::set_active);
	ClassDB::bind_method(D_METHOD("get_active"), &BTPlayer::get_active);
	ClassDB::bind_method(D_METHOD("set_blackboard", "p_blackboard"), &BTPlayer::set_blackboard);
	ClassDB::bind_method(D_METHOD("get_blackboard"), &BTPlayer::get_blackboard);
	ClassDB::bind_method(D_METHOD("set_prefetch_nodepath_vars", "p_value"), &BTPlayer::set_prefetch_nodepath_vars);
	ClassDB::bind_method(D_METHOD("get_prefetch_nodepath_vars"), &BTPlayer::get_prefetch_nodepath_vars);

	ClassDB::bind_method(D_METHOD("_set_blackboard_data", "p_blackboard"), &BTPlayer::_set_blackboard_data);
	ClassDB::bind_method(D_METHOD("_get_blackboard_data"), &BTPlayer::_get_blackboard_data);

	ClassDB::bind_method(D_METHOD("update", "p_delta"), &BTPlayer::update);
	ClassDB::bind_method(D_METHOD("restart"), &BTPlayer::restart);
	ClassDB::bind_method(D_METHOD("get_last_status"), &BTPlayer::get_last_status);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "behavior_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviorTree"), "set_behavior_tree", "get_behavior_tree");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "update_mode", PROPERTY_HINT_ENUM, "Idle,Physics,Manual"), "set_update_mode", "get_update_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "get_active");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "blackboard", PROPERTY_HINT_NONE, "Blackboard", 0), "set_blackboard", "get_blackboard");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "_blackboard_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_INTERNAL), "_set_blackboard_data", "_get_blackboard_data");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "prefetch_nodepath_vars"), "set_prefetch_nodepath_vars", "get_prefetch_nodepath_vars");

	BIND_ENUM_CONSTANT(IDLE);
	BIND_ENUM_CONSTANT(PHYSICS);
	BIND_ENUM_CONSTANT(MANUAL);

	ADD_SIGNAL(MethodInfo("behavior_tree_finished", PropertyInfo(Variant::INT, "p_status")));
	ADD_SIGNAL(MethodInfo("updated", PropertyInfo(Variant::INT, "p_status")));

#ifdef DEBUG_ENABLED
	ClassDB::bind_method(D_METHOD("_set_monitor_performance", "p_value"), &BTPlayer::_set_monitor_performance);
	ClassDB::bind_method(D_METHOD("_get_monitor_performance"), &BTPlayer::_get_monitor_performance);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "monitor_performance"), "_set_monitor_performance", "_get_monitor_performance");
	ADD_PROPERTY_DEFAULT("monitor_performance", false);
#endif // DEBUG_ENABLED
}

BTPlayer::BTPlayer() {
	blackboard = Ref<Blackboard>(memnew(Blackboard));
}

BTPlayer::~BTPlayer() {
}
