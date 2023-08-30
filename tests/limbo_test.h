/**
 * limbo_test.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBO_TEST_H
#define LIMBO_TEST_H

#include "tests/test_macros.h"

#include "modules/limboai/bt/tasks/bt_action.h"

class BTTestAction : public BTAction {
	GDCLASS(BTTestAction, BTAction);

public:
	int ret_status = BTTask::SUCCESS;
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
	bool is_status_either(int p_status1, int p_status2) { return (get_status() == p_status1 || get_status() == p_status2); }

	BTTestAction(int p_return_status) { ret_status = p_return_status; }
	BTTestAction() {}
};

#define CHECK_ENTRIES_TICKS_EXITS(m_task, m_entries, m_ticks, m_exits) \
	CHECK(m_task->num_entries == m_entries);                           \
	CHECK(m_task->num_ticks == m_ticks);                               \
	CHECK(m_task->num_exits == m_exits);

#define CHECK_ENTRIES_TICKS_EXITS_UP_TO(m_task, m_entries, m_ticks, m_exits) \
	CHECK(m_task->num_entries <= m_entries);                                 \
	CHECK(m_task->num_ticks <= m_ticks);                                     \
	CHECK(m_task->num_exits <= m_exits);

#endif // LIMBO_TEST_H
