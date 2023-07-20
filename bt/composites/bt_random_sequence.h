/* bt_random_sequence.h */

#ifndef BT_RANDOM_SEQUENCE_H
#define BT_RANDOM_SEQUENCE_H

#include "bt_composite.h"

#include "core/templates/vector.h"

class BTRandomSequence : public BTComposite {
	GDCLASS(BTRandomSequence, BTComposite);

private:
	int last_running_idx = 0;
	Array indicies;

protected:
	virtual void _enter() override;
	virtual int _tick(double p_delta) override;
};
#endif // BT_RANDOM_SEQUENCE_H