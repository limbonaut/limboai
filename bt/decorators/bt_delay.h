/* bt_delay.h */

#ifndef BT_DELAY_H
#define BT_DELAY_H

#include "bt_decorator.h"
#include "core/object/object.h"

class BTDelay : public BTDecorator {
	GDCLASS(BTDelay, BTDecorator);

private:
	double seconds = 1.0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual int _tick(double p_delta) override;

public:
	void set_seconds(double p_value) {
		seconds = p_value;
		emit_changed();
	}
	double get_seconds() const { return seconds; }
};

#endif // BT_DELAY_H