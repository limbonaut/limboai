/**
 * bt_repeat.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_REPEAT_H
#define BT_REPEAT_H

#include "bt_decorator.h"

#include "core/object/object.h"

class BTRepeat : public BTDecorator {
	GDCLASS(BTRepeat, BTDecorator);

private:
	int times = 1;
	bool abort_on_failure = false;
	int cur_iteration = 0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual void _enter() override;
	virtual int _tick(double p_delta) override;

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