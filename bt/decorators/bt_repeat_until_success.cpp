/* bt_repeat_until_success.cpp */

#include "bt_repeat_until_success.h"

int BTRepeatUntilSuccess::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	if (get_child(0)->execute(p_delta) == SUCCESS) {
		return SUCCESS;
	}
	return RUNNING;
}
