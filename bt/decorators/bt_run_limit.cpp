/* bt_run_limit.cpp */

#include "bt_run_limit.h"

String BTRunLimit::_generate_name() const {
	return vformat("RunLimit x%d", run_limit);
}

int BTRunLimit::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	if (get_child(0)->get_status() != RUNNING) {
		if (_num_runs >= run_limit) {
			return FAILURE;
		}
		_num_runs += 1;
	}
	return get_child(0)->execute(p_delta);
}

void BTRunLimit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_run_limit", "p_value"), &BTRunLimit::set_run_limit);
	ClassDB::bind_method(D_METHOD("get_run_limit"), &BTRunLimit::get_run_limit);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "run_limit"), "set_run_limit", "get_run_limit");
}
