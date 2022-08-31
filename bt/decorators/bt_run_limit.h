/* bt_run_limit.h */

#ifndef BT_RUN_LIMIT_H
#define BT_RUN_LIMIT_H

#include "bt_decorator.h"
#include "core/object.h"

class BTRunLimit : public BTDecorator {
	GDCLASS(BTRunLimit, BTDecorator);

private:
	int run_limit = 1;
	int _num_runs = 0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual int _tick(float p_delta);

public:
	void set_run_limit(int p_value) {
		run_limit = p_value;
		emit_changed();
	}
	int get_run_limit() const { return run_limit; }
};

#endif // BT_RUN_LIMIT_H