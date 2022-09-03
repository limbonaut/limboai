/* bt_subtree.cpp */

#include "bt_subtree.h"
#include "core/error_macros.h"
#include "core/object.h"
#include "core/variant.h"
#include "modules/limboai/bt/actions/bt_action.h"

String BTSubtree::_generate_name() const {
	return vformat("Subtree '%s'", subtree.is_null() ? "?" : subtree->get_path());
}

Ref<BTTask> BTSubtree::clone() const {
	ERR_FAIL_COND_V_MSG(!subtree.is_valid(), nullptr, vformat("Subtree is not valid (%s)", get_agent()));
	ERR_FAIL_COND_V_MSG(!subtree->get_root_task().is_valid(), nullptr, vformat("Subtree root task is not valid (%s)", get_agent()));
	return subtree->get_root_task()->clone();
}

String BTSubtree::get_configuration_warning() const {
	String warning = BTAction::get_configuration_warning();
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