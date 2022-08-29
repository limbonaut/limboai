/* bt_random_wait.h */

#ifndef BT_RANDOM_WAIT_H
#define BT_RANDOM_WAIT_H

#include "bt_action.h"
#include "core/object.h"

class BTRandomWait : public BTAction {
	GDCLASS(BTRandomWait, BTAction);

private:
	Vector2 duration_min_max = Vector2(1.0, 2.0);

	float _time_passed = 0.0;
	float _duration = 0.0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _enter();
	virtual int _tick(float p_delta);

public:
	void set_duration_min_max(Vector2 p_value) {
		duration_min_max = p_value;
		emit_changed();
	}
	Vector2 get_duration_min_max() const { return duration_min_max; }
};

#endif // BT_RANDOM_WAIT_H