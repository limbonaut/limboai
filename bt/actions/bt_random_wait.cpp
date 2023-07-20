/* bt_random_wait.cpp */

#include "bt_random_wait.h"

#include "core/math/math_funcs.h"

String BTRandomWait::_generate_name() const {
	return vformat("Wait %s to %s sec",
			Math::snapped(min_duration, 0.001),
			Math::snapped(max_duration, 0.001));
}

void BTRandomWait::_enter() {
	duration = Math::random(min_duration, max_duration);
}

int BTRandomWait::_tick(double p_delta) {
	if (get_elapsed_time() < duration) {
		return RUNNING;
	} else {
		return SUCCESS;
	}
}

void BTRandomWait::set_min_duration(double p_max_duration) {
	min_duration = p_max_duration;
	if (max_duration < min_duration) {
		set_max_duration(min_duration);
	}
	emit_changed();
}

void BTRandomWait::set_max_duration(double p_max_duration) {
	max_duration = p_max_duration;
	if (min_duration > max_duration) {
		set_min_duration(max_duration);
	}
	emit_changed();
}

void BTRandomWait::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_min_duration"), &BTRandomWait::set_min_duration);
	ClassDB::bind_method(D_METHOD("get_min_duration"), &BTRandomWait::get_min_duration);
	ClassDB::bind_method(D_METHOD("set_max_duration"), &BTRandomWait::set_max_duration);
	ClassDB::bind_method(D_METHOD("get_max_duration"), &BTRandomWait::get_max_duration);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_duration"), "set_min_duration", "get_min_duration");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_duration"), "set_max_duration", "get_max_duration");
}
