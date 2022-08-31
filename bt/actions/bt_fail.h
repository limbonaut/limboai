/* bt_fail.h */

#ifndef BT_FAIL_H
#define BT_FAIL_H

#include "bt_action.h"
#include "core/object.h"

class BTFail : public BTAction {
	GDCLASS(BTFail, BTAction);

protected:
	virtual int _tick(float p_delta);
};

#endif // BT_FAIL_H