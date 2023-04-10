/* bt_random_wait.h */

#ifndef BT_RANDOM_WAIT_H
#define BT_RANDOM_WAIT_H

#include "bt_action.h"
#include "core/object/object.h"

class BTRandomWait : public BTAction {
	GDCLASS(BTRandomWait, BTAction);

private:
	double min_duration = 1.0;
	double max_duration = 2.0;

	double time_passed = 0.0;
	double duration = 0.0;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual void _enter() override;
	virtual int _tick(double p_delta) override;

public:
	void set_min_duration(double p_max_duration);
	double get_min_duration() const { return min_duration; }

	void set_max_duration(double p_max_duration);
	double get_max_duration() const { return max_duration; }
};

#endif // BT_RANDOM_WAIT_H