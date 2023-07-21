/**
 * bt_action.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_ACTION_H
#define BT_ACTION_H

#include "../bt_task.h"

#include "core/object/object.h"

class BTAction : public BTTask {
	GDCLASS(BTAction, BTTask);

public:
	virtual String get_configuration_warning() const override;
};

#endif // BT_ACTION_H