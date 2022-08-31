/* bt_wait.h */

#ifndef BT_WAIT_H
#define BT_WAIT_H

#include "bt_action.h"
#include "core/object.h"

class BTWait : public BTAction {
	GDCLASS(BTWait, BTAction);

private:
	float duration = 1.0;

	float _time_passed = 0.0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _enter();
	virtual int _tick(float p_delta);

public:
	void set_duration(float p_value) {
		duration = p_value;
		emit_changed();
	}
	float get_duration() const { return duration; }
};

#endif // BT_WAIT_H