/* bt_repeat_until_success.h */

#ifndef BT_REPEAT_UNTIL_SUCCESS_H
#define BT_REPEAT_UNTIL_SUCCESS_H

#include "bt_decorator.h"
#include "core/object/object.h"

class BTRepeatUntilSuccess : public BTDecorator {
	GDCLASS(BTRepeatUntilSuccess, BTDecorator);

protected:
	virtual int _tick(float p_delta) override;
};

#endif // BT_REPEAT_UNTIL_SUCCESS_H