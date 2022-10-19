/* bt_state.cpp */

#include "bt_state.h"
#include "core/class_db.h"
#include "core/variant.h"
#include "modules/limboai/bt/bt_task.h"
#include "modules/limboai/limbo_state.h"

// void BTState::initialize(Object *p_agent, const Ref<Blackboard> &p_blackboard) {
void BTState::_setup() {
	// blackboard->prefetch_nodepath_vars(this);
	// blackboard->set_parent_scope(p_blackboard);
	root_task = behavior_tree->instance(get_agent(), get_blackboard());

	// LimboState::initialize(p_agent, blackboard);
}

void BTState::_exit() {
	root_task->cancel();
}

void BTState::_update(float p_delta) {
	int status = root_task->execute(p_delta);
	if (status == BTTask::SUCCESS) {
		get_root()->dispatch("success", Variant());
	} else if (status == BTTask::FAILURE) {
		get_root()->dispatch("failure", Variant());
	}
}

void BTState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_behavior_tree", "p_value"), &BTState::set_behavior_tree);
	ClassDB::bind_method(D_METHOD("get_behavior_tree"), &BTState::get_behavior_tree);
	// ClassDB::bind_method(D_METHOD("_get_blackboard_data"), &BTState::_get_blackboard_data);
	// ClassDB::bind_method(D_METHOD("_set_blackboard_data", "p_data"), &BTState::_set_blackboard_data);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "behavior_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviorTree"), "set_behavior_tree", "get_behavior_tree");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "blackboard_data"), "_set_blackboard_data", "_get_blackboard_data");
}

BTState::BTState() {
	// blackboard = Ref<Blackboard>(memnew(Blackboard));
}
