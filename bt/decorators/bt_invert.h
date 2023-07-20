/* bt_invert.h */

#ifndef BT_INVERT_H
#define BT_INVERT_H

#include "bt_decorator.h"

#include "core/object/object.h"

class BTInvert : public BTDecorator {
	GDCLASS(BTInvert, BTDecorator);

protected:
	virtual int _tick(double p_delta) override;
};

#endif // BT_INVERT_H