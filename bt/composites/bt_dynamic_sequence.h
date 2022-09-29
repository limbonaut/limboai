/* bt_dynamic_sequence.h */

#ifndef BT_DYNAMIC_SEQUENCE_H
#define BT_DYNAMIC_SEQUENCE_H

#include "bt_composite.h"
#include "core/object.h"

class BTDynamicSequence : public BTComposite {
	GDCLASS(BTDynamicSequence, BTComposite);

private:
	int last_running_idx = 0;

protected:
	virtual void _enter();
	virtual int _tick(float p_delta);
};

#endif // BT_DYNAMIC_SEQUENCE_H