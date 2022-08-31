/* bt_random_wait.cpp */

#include "bt_random_wait.h"

String BTRandomWait::_generate_name() const {
	return vformat("Wait %s to %s sec", duration_min_max.x, duration_min_max.y);
}

void BTRandomWait::_enter() {
	_time_passed = 0.0;
	_duration = Math::random(duration_min_max.x, duration_min_max.y);
}

int BTRandomWait::_tick(float p_delta) {
	_time_passed += p_delta;
	if (_time_passed < _duration) {
		return RUNNING;
	} else {
		return SUCCESS;
	}
}

void BTRandomWait::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_duration_min_max", "p_value"), &BTRandomWait::set_duration_min_max);
	ClassDB::bind_method(D_METHOD("get_duration_min_max"), &BTRandomWait::get_duration_min_max);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "duration_min_max"), "set_duration_min_max", "get_duration_min_max");
}
