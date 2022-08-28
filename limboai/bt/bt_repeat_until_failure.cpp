/* bt_repeat_until_failure.cpp */

#include "bt_repeat_until_failure.h"

int BTRepeatUntilFailure::_tick(float p_delta) {
	if (get_child(0)->execute(p_delta) == FAILURE) {
		return SUCCESS;
	}
	return RUNNING;
}
