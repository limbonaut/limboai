/* bt_subtree.cpp */

#include "bt_subtree.h"
#include "core/engine.h"
#include "core/error_macros.h"
#include "core/object.h"
#include "core/typedefs.h"
#include "core/variant.h"
#include "modules/limboai/blackboard.h"
#include "modules/limboai/bt/actions/bt_action.h"
#include "modules/limboai/bt/actions/bt_fail.h"
#include "modules/limboai/bt/bt_task.h"
#include "modules/limboai/bt/decorators/bt_decorator.h"
#include "modules/limboai/bt/decorators/bt_new_scope.h"

String BTSubtree::_generate_name() const {
	String s;
	if (subtree.is_null()) {
		s = "(unassigned)";
	} else if (subtree->get_path().empty()) {
		s = "(unsaved)";
	} else {
		s = vformat("\"%s\"", subtree->get_path());
	}
	return vformat("Subtree %s", s);
}

void BTSubtree::initialize(Object *p_agent, const Ref<Blackboard> &p_blackboard) {
	ERR_FAIL_COND_MSG(!subtree.is_valid(), "Subtree is not assigned.");
	ERR_FAIL_COND_MSG(!subtree->get_root_task().is_valid(), "Subtree root task is not valid.");
	ERR_FAIL_COND_MSG(get_child_count() != 0, "Subtree task shouldn't have children during initialization.");

	// while (get_child_count() > 0) {
	// 	remove_child_at_index(get_child_count() - 1);
	// }

	add_child(subtree->get_root_task()->clone());

	BTNewScope::initialize(p_agent, p_blackboard);
}

int BTSubtree::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator doesn't have a child.");
	return get_child(0)->execute(p_delta);
}

String BTSubtree::get_configuration_warning() const {
	String warning = BTTask::get_configuration_warning(); // BTDecorator skipped intentionally
	if (!warning.empty()) {
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