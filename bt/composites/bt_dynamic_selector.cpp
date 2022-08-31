/* bt_dynamic_selector.cpp */

#include "bt_dynamic_selector.h"

void BTDynamicSelector::_enter() {
	last_running_idx = 0;
}

int BTDynamicSelector::_tick(float p_delta) {
	int status = SUCCESS;
	int i;
	for (i = 0; i < get_child_count(); i++) {
		status = get_child(i)->execute(p_delta);
		if (status != FAILURE) {
			break;
		}
	}
	// If the last node ticked is earlier in the tree than the previous runner,
	// cancel previous runner.
	if (last_running_idx > i && get_child(last_running_idx)->get_status() == RUNNING) {
		get_child(last_running_idx)->cancel();
	}
	last_running_idx = i;
	return status;
}