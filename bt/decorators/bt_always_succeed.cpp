/* bt_always_succeed.cpp */

#include "bt_always_succeed.h"

int BTAlwaysSucceed::_tick(double p_delta) {
	if (get_child_count() > 0 && get_child(0)->execute(p_delta) == RUNNING) {
		return RUNNING;
	}
	return SUCCESS;
}
