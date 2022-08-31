/* bt_condition.h */

#ifndef BT_CONDITION_H
#define BT_CONDITION_H

#include "../bt_task.h"
#include "core/object.h"

class BTCondition : public BTTask {
	GDCLASS(BTCondition, BTTask);

public:
	virtual String get_configuration_warning() const;
};

#endif // BT_CONDITION_H