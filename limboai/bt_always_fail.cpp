/* bt_always_fail.cpp */

#include "bt_always_fail.h"

int BTAlwaysFail::_tick(float p_delta) {
	if (get_child(0)->execute(p_delta) == RUNNING) {
		return RUNNING;
	}
	return FAILURE;
}
