/* bt_for_each.cpp */

#include "bt_for_each.h"
#include "core/error_list.h"
#include "core/error_macros.h"
#include "core/variant.h"
#include "modules/limboai/blackboard.h"
#include "modules/limboai/limbo_utility.h"

String BTForEach::_generate_name() const {
	return vformat("ForEach %s in %s",
			LimboUtility::get_singleton()->decorate_var(save_var),
			LimboUtility::get_singleton()->decorate_var(array_var));
}

void BTForEach::_enter() {
	current_idx = 0;
}

int BTForEach::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "ForEach decorator has no child.");
	ERR_FAIL_COND_V_MSG(save_var.empty(), FAILURE, "ForEach save variable is not set.");
	ERR_FAIL_COND_V_MSG(array_var.empty(), FAILURE, "ForEach array variable is not set.");

	Array arr = get_blackboard()->get_var(array_var, Variant());
	if (arr.size() == 0) {
		return SUCCESS;
	}
	Variant elem = arr.get(current_idx);
	get_blackboard()->set_var(save_var, elem);

	int status = get_child(0)->execute(p_delta);
	if (status == RUNNING) {
		return RUNNING;
	} else if (status == FAILURE) {
		return FAILURE;
	} else if (current_idx == (arr.size() - 1)) {
		return SUCCESS;
	} else {
		current_idx += 1;
		return RUNNING;
	}
}

void BTForEach::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_array_var", "p_variable"), &BTForEach::set_array_var);
	ClassDB::bind_method(D_METHOD("get_array_var"), &BTForEach::get_array_var);
	ClassDB::bind_method(D_METHOD("set_save_var", "p_variable"), &BTForEach::set_save_var);
	ClassDB::bind_method(D_METHOD("get_save_var"), &BTForEach::get_save_var);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "array_var"), "set_array_var", "get_array_var");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "save_var"), "set_save_var", "get_save_var");
}