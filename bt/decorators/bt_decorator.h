/* bt_decorator.h */

#ifndef BT_DECORATOR_H
#define BT_DECORATOR_H

#include "../bt_task.h"

#include "core/object/object.h"

class BTDecorator : public BTTask {
	GDCLASS(BTDecorator, BTTask)

public:
	virtual String get_configuration_warning() const override;
};

#endif // BT_DECORATOR_H