/**
 * bt_wait.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_wait.h"

#include "core/math/math_funcs.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/variant.h"

String BTWait::_generate_name() const {
	return vformat("Wait %s sec", Math::snapped(duration, 0.001));
}

BT::Status BTWait::_tick(double p_delta) {
	if (get_elapsed_time() < duration) {
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
