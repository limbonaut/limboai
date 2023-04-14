/* bt_time_limit.cpp */

#include "bt_time_limit.h"
#include "core/math/math_funcs.h"

String BTTimeLimit::_generate_name() const {
	return vformat("TimeLimit %s sec", Math::snapped(time_limit, 0.001));
}

int BTTimeLimit::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	int status = get_child(0)->execute(p_delta);
	if (status == RUNNING and get_elapsed_time() >= time_limit) {
		get_child(0)->cancel();
		return FAILURE;
	}
	return status;
}

void BTTimeLimit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_time_limit", "p_value"), &BTTimeLimit::set_time_limit);
	ClassDB::bind_method(D_METHOD("get_time_limit"), &BTTimeLimit::get_time_limit);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_limit"), "set_time_limit", "get_time_limit");
}
