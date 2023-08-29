/**
 * bt_test_action.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_TEST_ACTION_H
#define BT_TEST_ACTION_H

#include "modules/limboai/bt/tasks/bt_action.h"

class BTTestAction : public BTAction {
	GDCLASS(BTTestAction, BTAction);

private:
	int ret_status = BTTask::SUCCESS;

public:
	int num_entries = 0;
	int num_ticks = 0;
	int num_exits = 0;

protected:
	virtual void _enter() override { num_entries += 1; }
	virtual void _exit() override { num_exits += 1; }

	virtual int _tick(double p_delta) override {
		num_ticks += 1;
		return ret_status;
	}

public:
	BTTestAction(int p_return_status) { ret_status = p_return_status; }
};

#endif // BT_TEST_ACTION_H
