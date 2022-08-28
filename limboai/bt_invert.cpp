/* bt_invert.cpp */

#include "bt_invert.h"

int BTInvert::_tick(float p_delta) {
	int status = get_child(0)->execute(p_delta);
	if (status == SUCCESS) {
		status = FAILURE;
	} else if (status == FAILURE) {
		status = SUCCESS;
	}
	return status;
}