/* bt_always_fail.h */

#ifndef BT_ALWAYS_FAIL_H
#define BT_ALWAYS_FAIL_H

#include "bt_decorator.h"
#include "core/object/object.h"

class BTAlwaysFail : public BTDecorator {
	GDCLASS(BTAlwaysFail, BTDecorator);

protected:
	virtual int _tick(double p_delta) override;
};

#endif // BT_ALWAYS_FAIL_H