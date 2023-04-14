/* bt_delay.cpp */

#include "bt_delay.h"
#include "core/error/error_macros.h"
#include "core/math/math_funcs.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"

String BTDelay::_generate_name() const {
	return vformat("Delay %s sec", Math::snapped(seconds, 0.001));
}

int BTDelay::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	if (get_elapsed_time() <= seconds) {
		return RUNNING;
	}
	return get_child(0)->execute(p_delta);
}

void BTDelay::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_seconds", "p_value"), &BTDelay::set_seconds);
	ClassDB::bind_method(D_METHOD("get_seconds"), &BTDelay::get_seconds);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "seconds"), "set_seconds", "get_seconds");
}
