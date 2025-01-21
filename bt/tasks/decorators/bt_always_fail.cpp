/**
 * bt_always_fail.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_always_fail.h"

BT::Status BTAlwaysFail::_tick(double p_delta) {
	if (get_child_count() > 0 && get_child(0)->execute(p_delta) == RUNNING) {
		return RUNNING;
	}
	return FAILURE;
}
