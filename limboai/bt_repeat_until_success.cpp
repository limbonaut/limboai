/* bt_repeat_until_success.cpp */

#include "bt_repeat_until_success.h"

int BTRepeatUntilSuccess::_tick(float p_delta) {
	if (get_child(0)->execute(p_delta) == SUCCESS) {
		return SUCCESS;
	}
	return RUNNING;
}
