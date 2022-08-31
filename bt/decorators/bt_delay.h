/* bt_delay.h */

#ifndef BT_DELAY_H
#define BT_DELAY_H

#include "bt_decorator.h"
#include "core/object.h"

class BTDelay : public BTDecorator {
	GDCLASS(BTDelay, BTDecorator);

private:
	float seconds = 1.0;
	float time_passed = 0.0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _enter();
	virtual int _tick(float p_delta);

public:
	void set_seconds(float p_value) {
		seconds = p_value;
		emit_changed();
	}
	float get_seconds() const { return seconds; }
};

#endif // BT_DELAY_H