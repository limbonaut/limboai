/* bt_sequence.cpp */

#include "bt_sequence.h"

void BTSequence::_enter() {
	last_running_idx = 0;
}

int BTSequence::_tick(float p_delta) {
	int status = FRESH;
	for (int i = 0; i < get_child_count(); i++) {
		status = get_child(i)->execute(p_delta);
		if (status != SUCCESS) {
			last_running_idx = i;
			break;
		}
	}
	return status;
}