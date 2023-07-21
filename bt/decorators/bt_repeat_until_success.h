/**
 * bt_repeat_until_success.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_REPEAT_UNTIL_SUCCESS_H
#define BT_REPEAT_UNTIL_SUCCESS_H

#include "bt_decorator.h"

#include "core/object/object.h"

class BTRepeatUntilSuccess : public BTDecorator {
	GDCLASS(BTRepeatUntilSuccess, BTDecorator);

protected:
	virtual int _tick(double p_delta) override;
};

#endif // BT_REPEAT_UNTIL_SUCCESS_H