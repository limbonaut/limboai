/* bt_random_selector.h */

#ifndef BT_RANDOM_SELECTOR_H
#define BT_RANDOM_SELECTOR_H

#include "bt_composite.h"
#include "core/vector.h"

class BTRandomSelector : public BTComposite {
	GDCLASS(BTRandomSelector, BTComposite);

private:
	int last_running_idx = 0;
	Array _indicies;

protected:
	virtual void _enter();
	virtual int _tick(float p_delta);
};
#endif // BT_RANDOM_SELECTOR_H