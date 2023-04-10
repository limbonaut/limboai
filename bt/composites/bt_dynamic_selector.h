/* bt_dynamic_selector.h */

#ifndef BT_DYNAMIC_SELECTOR_H
#define BT_DYNAMIC_SELECTOR_H

#include "bt_composite.h"
#include "core/object/object.h"

class BTDynamicSelector : public BTComposite {
	GDCLASS(BTDynamicSelector, BTComposite);

private:
	int last_running_idx = 0;

protected:
	virtual void _enter() override;
	virtual int _tick(double p_delta) override;
};

#endif // BT_DYNAMIC_SELECTOR_H