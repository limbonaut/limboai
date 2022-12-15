/* bt_always_succeed.h */

#ifndef BT_ALWAYS_SUCCEED_H
#define BT_ALWAYS_SUCCEED_H

#include "bt_decorator.h"
#include "core/object/object.h"

class BTAlwaysSucceed : public BTDecorator {
	GDCLASS(BTAlwaysSucceed, BTDecorator);

protected:
	virtual int _tick(float p_delta) override;
};

#endif // BT_ALWAYS_SUCCEED_H