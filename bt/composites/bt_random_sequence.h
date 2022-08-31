/* bt_random_sequence.h */

#ifndef BT_RANDOM_SEQUENCE_H
#define BT_RANDOM_SEQUENCE_H

#include "bt_composite.h"
#include "core/vector.h"

class BTRandomSequence : public BTComposite {
	GDCLASS(BTRandomSequence, BTComposite);

private:
	int last_running_idx = 0;
	Array _indicies;

protected:
	virtual void _enter();
	virtual int _tick(float p_delta);
};
#endif // BT_RANDOM_SEQUENCE_H