/* bt_parallel.h */

#ifndef BT_PARALLEL_H
#define BT_PARALLEL_H

#include "bt_composite.h"
#include "core/object.h"

class BTParallel : public BTComposite {
	GDCLASS(BTParallel, BTComposite);

private:
	int num_successes_required = 1;
	int num_failures_required = 1;
	bool repeat = false;

protected:
	static void _bind_methods();

	virtual void _enter();
	virtual int _tick(float p_delta);

public:
	int get_num_successes_required() const { return num_successes_required; }
	void set_num_successes_required(int p_value) {
		num_successes_required = p_value;
		emit_changed();
	}
	int get_num_failures_required() const { return num_failures_required; }
	void set_num_failures_required(int p_value) {
		num_failures_required = p_value;
		emit_changed();
	}
	bool get_repeat() const { return repeat; }
	void set_repeat(bool p_value) {
		repeat = p_value;
		emit_changed();
	}
};

#endif // BT_PARALLEL_H