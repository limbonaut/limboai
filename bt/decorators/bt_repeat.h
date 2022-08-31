/* bt_repeat.h */

#ifndef BT_REPEAT_H
#define BT_REPEAT_H

#include "bt_decorator.h"
#include "core/object.h"

class BTRepeat : public BTDecorator {
	GDCLASS(BTRepeat, BTDecorator);

private:
	int times = 1;
	bool abort_on_failure = false;
	int _cur_iteration = 0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _enter();
	virtual int _tick(float p_delta);

public:
	void set_times(int p_value) {
		times = p_value;
		emit_changed();
	}
	int get_times() const { return times; }
	void set_abort_on_failure(bool p_value) {
		abort_on_failure = p_value;
		emit_changed();
	}
	bool get_abort_on_failure() const { return abort_on_failure; }
};

#endif // BT_REPEAT_H