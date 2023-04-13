/* bt_state.cpp */

#include "bt_state.h"
#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/variant/variant.h"
#include "modules/limboai/bt/bt_task.h"
#include "modules/limboai/debugger/limbo_debugger.h"
#include "modules/limboai/limbo_state.h"
#include "modules/limboai/limbo_string_names.h"

void BTState::_setup() {
	ERR_FAIL_COND_MSG(behavior_tree.is_null(), "BTState: BehaviorTree is not assigned.");
	tree_instance = behavior_tree->instantiate(get_agent(), get_blackboard());

#ifdef DEBUG_ENABLED
	LimboDebugger::get_singleton()->register_bt_instance(tree_instance, get_path());
#endif
}

void BTState::_exit() {
	ERR_FAIL_COND(tree_instance == nullptr);
	tree_instance->cancel();
}

void BTState::_update(double p_delta) {
	ERR_FAIL_COND(tree_instance == nullptr);
	int status = tree_instance->execute(p_delta);
	emit_signal(LimboStringNames::get_singleton()->updated, p_delta);
	if (status == BTTask::SUCCESS) {
		get_root()->dispatch(success_event, Variant());
	} else if (status == BTTask::FAILURE) {
		get_root()->dispatch(failure_event, Variant());
	}
}

#ifdef DEBUG_ENABLED
void BTState::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_EXIT_TREE: {
			LimboDebugger::get_singleton()->unregister_bt_instance(tree_instance, get_path());
		} break;
	}
}
#endif // DEBUG_ENABLED

void BTState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_behavior_tree", "p_value"), &BTState::set_behavior_tree);
	ClassDB::bind_method(D_METHOD("get_behavior_tree"), &BTState::get_behavior_tree);

	ClassDB::bind_method(D_METHOD("set_success_event", "p_event_name"), &BTState::set_success_event);
	ClassDB::bind_method(D_METHOD("get_success_event"), &BTState::get_success_event);

	ClassDB::bind_method(D_METHOD("set_failure_event", "p_event_name"), &BTState::set_failure_event);
	ClassDB::bind_method(D_METHOD("get_failure_event"), &BTState::get_failure_event);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "behavior_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviorTree"), "set_behavior_tree", "get_behavior_tree");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "success_event"), "set_success_event", "get_success_event");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "failure_event"), "set_failure_event", "get_failure_event");
}

BTState::BTState() {
	success_event = "success";
	failure_event = "failure";
}
