/**
 * bt_sequence.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_SEQUENCE_H
#define BT_SEQUENCE_H

#include "../bt_composite.h"

class BTSequence : public BTComposite {
	GDCLASS(BTSequence, BTComposite);
	TASK_CATEGORY(Composites);

private:
	int last_running_idx = 0;

protected:
	static void _bind_methods() {}

	virtual void _enter() override;
	virtual Status _tick(double p_delta) override;
};

#endif // BT_SEQUENCE_H
