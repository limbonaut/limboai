/**
 * bt_composite.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_COMPOSITE_H
#define BT_COMPOSITE_H

#include "bt_task.h"

class BTComposite : public BTTask {
	GDCLASS(BTComposite, BTTask);

protected:
	static void _bind_methods() {}

public:
	virtual PackedStringArray get_configuration_warnings() override;
};

#endif // BT_COMPOSITE_H
