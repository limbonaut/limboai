/**
 * bt_comment.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_COMMENT_H
#define BT_COMMENT_H

#include "bt_task.h"

class BTComment : public BTTask {
	GDCLASS(BTComment, BTTask);
	TASK_CATEGORY(Utility);

protected:
	static void _bind_methods() {}

public:
	virtual void set_enabled(bool p_enabled) override;
	virtual Ref<BTTask> clone() const override;
	virtual PackedStringArray get_configuration_warnings() override;

	BTComment();
};

#endif // BT_COMMENT_H
