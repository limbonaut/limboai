/* bt_random_sequence.cpp */

#include "bt_random_sequence.h"

void BTRandomSequence::_enter() {
	last_running_idx = 0;
	if (_indicies.size() != get_child_count()) {
		_indicies.resize(get_child_count());
		for (int i = 0; i < get_child_count(); i++) {
			_indicies.set(i, i);
		}
	}
	_indicies.shuffle();
}

int BTRandomSequence::_tick(float p_delta) {
	int status = SUCCESS;
	for (int i = last_running_idx; i < get_child_count(); i++) {
		status = get_child(_indicies[i])->execute(p_delta);
		if (status != SUCCESS) {
			last_running_idx = i;
			break;
		}
	}
	return status;
}