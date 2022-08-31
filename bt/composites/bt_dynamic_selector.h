/* bt_dynamic_selector.h */

#ifndef BT_DYNAMIC_SELECTOR_H
#define BT_DYNAMIC_SELECTOR_H

#import "bt_composite.h"
#include "core/object.h"

class BTDynamicSelector : public BTComposite {
	GDCLASS(BTDynamicSelector, BTComposite);

private:
	int last_running_idx = 0;

protected:
	virtual void _enter();
	virtual int _tick(float p_delta);
};

#endif // BT_DYNAMIC_SELECTOR_H