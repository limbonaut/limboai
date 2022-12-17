/* bt_wait.cpp */

#include "bt_wait.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/variant.h"

String BTWait::_generate_name() const {
	return vformat("Wait %ss", duration);
}

void BTWait::_enter() {
	time_passed = 0.0;
}

int BTWait::_tick(float p_delta) {
	time_passed += p_delta;
	if (time_passed < duration) {
		return RUNNING;
	} else {
		return SUCCESS;
	}
}

void BTWait::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_duration", "p_value"), &BTWait::set_duration);
	ClassDB::bind_method(D_METHOD("get_duration"), &BTWait::get_duration);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"), "set_duration", "get_duration");
}
