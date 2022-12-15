/* bt_composite.h */

#ifndef BT_COMPOSITE_H
#define BT_COMPOSITE_H

#include "../bt_task.h"
#include "core/object/object.h"

class BTComposite : public BTTask {
	GDCLASS(BTComposite, BTTask);

public:
	virtual String get_configuration_warning() const override;
};

#endif // BT_COMPOSITE_H