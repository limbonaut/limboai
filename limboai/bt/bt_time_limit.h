/* bt_time_limit.h */

#ifndef BT_TIME_LIMIT_H
#define BT_TIME_LIMIT_H

#include "bt_decorator.h"
#include "core/object.h"

class BTTimeLimit : public BTDecorator {
	GDCLASS(BTTimeLimit, BTDecorator);

private:
	float time_limit = 5.0;
	float _time_passed = 0.0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _enter();
	virtual int _tick(float p_delta);

public:
	void set_time_limit(float p_value) {
		time_limit = p_value;
		emit_changed();
	};
	float get_time_limit() const { return time_limit; };
};

#endif // BT_TIME_LIMIT_H