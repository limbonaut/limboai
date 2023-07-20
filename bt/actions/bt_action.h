/* bt_action.h */

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