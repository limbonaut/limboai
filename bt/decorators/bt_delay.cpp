/* bt_delay.cpp */

#include "bt_delay.h"
#include "core/array.h"
#include "core/class_db.h"
#include "core/error_macros.h"
#include "core/object.h"
#include "core/variant.h"

String BTDelay::_generate_name() const {
	return vformat("Delay %ss", seconds);
}

void BTDelay::_enter() {
	time_passed = 0.0;
}

int BTDelay::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	time_passed += p_delta;
	if (time_passed <= seconds) {
		return RUNNING;
	}
	return get_child(0)->execute(p_delta);
}

void BTDelay::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_seconds", "p_value"), &BTDelay::set_seconds);
	ClassDB::bind_method(D_METHOD("get_seconds"), &BTDelay::get_seconds);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "seconds"), "set_seconds", "get_seconds");
}
