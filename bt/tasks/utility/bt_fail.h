/**
 * bt_fail.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_FAIL_H
#define BT_FAIL_H

#include "../bt_action.h"

class BTFail : public BTAction {
	GDCLASS(BTFail, BTAction);
	TASK_CATEGORY(Utility);

protected:
	static void _bind_methods() {}

	virtual Status _tick(double p_delta) override;
};

#endif // BT_FAIL_H
