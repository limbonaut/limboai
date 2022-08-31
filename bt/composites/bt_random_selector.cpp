/* bt_random_selector.cpp */

#include "bt_random_selector.h"

void BTRandomSelector::_enter() {
	last_running_idx = 0;
	if (_indicies.size() != get_child_count()) {
		_indicies.resize(get_child_count());
		for (int i = 0; i < get_child_count(); i++) {
			_indicies.set(i, i);
		}
	}
	_indicies.shuffle();
}

int BTRandomSelector::_tick(float p_delta) {
	int status = FAILURE;
	for (int i = 0; i < get_child_count(); i++) {
		status = get_child(_indicies[i])->execute(p_delta);
		if (status != FAILURE) {
			last_running_idx = i;
			break;
		}
	}
	return status;
}