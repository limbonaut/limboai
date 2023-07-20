/* bt_subtree.cpp */

#include "bt_subtree.h"

#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/bt/actions/bt_action.h"
#include "modules/limboai/bt/actions/bt_fail.h"
#include "modules/limboai/bt/bt_task.h"
#include "modules/limboai/bt/decorators/bt_decorator.h"
#include "modules/limboai/bt/decorators/bt_new_scope.h"

#include "core/config/engine.h"
#include "core/error/error_macros.h"
#include "core/object/object.h"
#include "core/typedefs.h"
#include "core/variant/variant.h"

String BTSubtree::_generate_name() const {
	String s;
	if (subtree.is_null()) {
		s = "(unassigned)";
	} else if (subtree->get_path().is_empty()) {
		s = "(unsaved)";
	} else {
		s = vformat("\"%s\"", subtree->get_path());
	}
	return vformat("Subtree %s", s);
}

void BTSubtree::initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard) {
	ERR_FAIL_COND_MSG(!subtree.is_valid(), "Subtree is not assigned.");
	ERR_FAIL_COND_MSG(!subtree->get_root_task().is_valid(), "Subtree root task is not valid.");
	ERR_FAIL_COND_MSG(get_child_count() != 0, "Subtree task shouldn't have children during initialization.");

	add_child(subtree->get_root_task()->clone());

	BTNewScope::initialize(p_agent, p_blackboard);
}

int BTSubtree::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator doesn't have a child.");
	return get_child(0)->execute(p_delta);
}

String BTSubtree::get_configuration_warning() const {
	String warning = BTTask::get_configuration_warning(); // BTDecorator skipped intentionally
	if (!warning.is_empty()) {
		warning += "\n";
	}
	if (subtree.is_null()) {
		warning += "Subtree needs to be assigned.\n";
	}
	return warning;
}

void BTSubtree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_subtree", "p_value"), &BTSubtree::set_subtree);
	ClassDB::bind_method(D_METHOD("get_subtree"), &BTSubtree::get_subtree);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "subtree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviorTree"), "set_subtree", "get_subtree");
}