/* bt_invert.h */

#ifndef BT_INVERT_H
#define BT_INVERT_H

#include "bt_decorator.h"
#include "core/object.h"

class BTInvert : public BTDecorator {
	GDCLASS(BTInvert, BTDecorator);

protected:
	virtual int _tick(float p_delta);
};

#endif // BT_INVERT_H