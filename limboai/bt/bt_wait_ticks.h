/* bt_wait_num_ticks.h */

#ifndef BT_WAIT_TICKS_H
#define BT_WAIT_TICKS_H

#include "bt_action.h"
#include "core/object.h"

class BTWaitTicks : public BTAction {
	GDCLASS(BTWaitTicks, BTAction);

private:
	int num_ticks = 1;

	int _num_passed = 0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _enter();
	virtual int _tick(float p_delta);

public:
	void set_num_ticks(int p_value) {
		num_ticks = p_value;
		emit_changed();
	}
	int get_num_ticks() const { return num_ticks; }
};

#endif // BT_WAIT_TICKS_H