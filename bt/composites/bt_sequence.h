/* bt_sequence.h */

#ifndef BT_SEQUENCE_H
#define BT_SEQUENCE_H

#include "bt_composite.h"

#include "core/object/object.h"

class BTSequence : public BTComposite {
	GDCLASS(BTSequence, BTComposite);

private:
	int last_running_idx = 0;

protected:
	virtual void _enter() override;
	virtual int _tick(double p_delta) override;
};

#endif // BT_SEQUENCE_H