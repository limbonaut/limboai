/* bt_condition.h */

#ifndef BT_CONDITION_H
#define BT_CONDITION_H

#include "../bt_task.h"

#include "core/object/object.h"

class BTCondition : public BTTask {
	GDCLASS(BTCondition, BTTask);

public:
	virtual String get_configuration_warning() const override;
};

#endif // BT_CONDITION_H