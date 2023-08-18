/**
 * bt_comment.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
/* bt_comment.h */

#ifndef BT_COMMENT_H
#define BT_COMMENT_H

#include "bt_task.h"

class BTComment : public BTTask {
	GDCLASS(BTComment, BTTask);

private:
public:
	virtual Ref<BTTask> clone() const override;
};

#endif // BT_COMMENT