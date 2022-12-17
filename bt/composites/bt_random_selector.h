/* bt_random_selector.h */

#ifndef BT_RANDOM_SELECTOR_H
#define BT_RANDOM_SELECTOR_H

#include "bt_composite.h"
#include "core/templates/vector.h"

class BTRandomSelector : public BTComposite {
	GDCLASS(BTRandomSelector, BTComposite);

private:
	int last_running_idx = 0;
	Array indicies;

protected:
	virtual void _enter() override;
	virtual int _tick(float p_delta) override;
};
#endif // BT_RANDOM_SELECTOR_H