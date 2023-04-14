/* bt_time_limit.h */

#ifndef BT_TIME_LIMIT_H
#define BT_TIME_LIMIT_H

#include "bt_decorator.h"
#include "core/object/object.h"

class BTTimeLimit : public BTDecorator {
	GDCLASS(BTTimeLimit, BTDecorator);

private:
	double time_limit = 5.0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual int _tick(double p_delta) override;

public:
	void set_time_limit(double p_value) {
		time_limit = p_value;
		emit_changed();
	}
	double get_time_limit() const { return time_limit; }
};

#endif // BT_TIME_LIMIT_H