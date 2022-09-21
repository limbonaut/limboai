/* bt_invert.cpp */

#include "bt_invert.h"

int BTInvert::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	int status = get_child(0)->execute(p_delta);
	if (status == SUCCESS) {
		status = FAILURE;
	} else if (status == FAILURE) {
		status = SUCCESS;
	}
	return status;
}