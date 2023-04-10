/* bt_random_selector.cpp */

#include "bt_random_selector.h"

void BTRandomSelector::_enter() {
	last_running_idx = 0;
	if (indicies.size() != get_child_count()) {
		indicies.resize(get_child_count());
		for (int i = 0; i < get_child_count(); i++) {
			indicies.set(i, i);
		}
	}
	indicies.shuffle();
}

int BTRandomSelector::_tick(double p_delta) {
	int status = FAILURE;
	for (int i = last_running_idx; i < get_child_count(); i++) {
		status = get_child(indicies[i])->execute(p_delta);
		if (status != FAILURE) {
			last_running_idx = i;
			break;
		}
	}
	return status;
}