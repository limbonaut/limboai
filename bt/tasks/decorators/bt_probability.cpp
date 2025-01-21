/**
 * bt_probability.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_probability.h"

#include "../../../util/limbo_compat.h"

void BTProbability::set_run_chance(float p_value) {
	run_chance = p_value;
	emit_changed();
}

String BTProbability::_generate_name() {
	return vformat("Probability %.1f%%", run_chance * 100.0);
}

BT::Status BTProbability::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	if (get_child(0)->get_status() == RUNNING || RANDF() <= run_chance) {
		return get_child(0)->execute(p_delta);
	}
	return FAILURE;
}

void BTProbability::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_run_chance", "probability"), &BTProbability::set_run_chance);
	ClassDB::bind_method(D_METHOD("get_run_chance"), &BTProbability::get_run_chance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "run_chance", PROPERTY_HINT_RANGE, "0.0,1.0"), "set_run_chance", "get_run_chance");
}
