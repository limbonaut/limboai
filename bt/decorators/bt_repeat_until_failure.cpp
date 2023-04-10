/* bt_repeat_until_failure.cpp */

#include "bt_repeat_until_failure.h"

int BTRepeatUntilFailure::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	if (get_child(0)->execute(p_delta) == FAILURE) {
		return SUCCESS;
	}
	return RUNNING;
}
