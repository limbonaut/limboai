/* bt_time_limit.cpp */

#include "bt_time_limit.h"

String BTTimeLimit::_generate_name() const {
	return vformat("TimeLimit %ss", time_limit);
}

void BTTimeLimit::_enter() {
	_time_passed = 0.0;
}

int BTTimeLimit::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	_time_passed += p_delta;
	int status = get_child(0)->execute(p_delta);
	if (status == RUNNING and _time_passed >= time_limit) {
		get_child(0)->cancel();
		return FAILURE;
	}
	return status;
}

void BTTimeLimit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_time_limit", "p_value"), &BTTimeLimit::set_time_limit);
	ClassDB::bind_method(D_METHOD("get_time_limit"), &BTTimeLimit::get_time_limit);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "time_limit"), "set_time_limit", "get_time_limit");
}
