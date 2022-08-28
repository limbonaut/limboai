/* bt_selector.cpp */

#include "bt_selector.h"

void BTSelector::_enter() {
	last_running_idx = 0;
}

int BTSelector::_tick(float p_delta) {
	int status = FRESH;
	for (int i = 0; i < get_child_count(); i++) {
		status = get_child(i)->execute(p_delta);
		if (status != FAILURE) {
			last_running_idx = i;
			break;
		}
	}
	return status;
}