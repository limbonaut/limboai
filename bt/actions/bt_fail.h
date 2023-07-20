/* bt_fail.h */

#ifndef BT_FAIL_H
#define BT_FAIL_H

#include "bt_action.h"

#include "core/object/object.h"

class BTFail : public BTAction {
	GDCLASS(BTFail, BTAction);

protected:
	virtual int _tick(double p_delta) override;
};

#endif // BT_FAIL_H