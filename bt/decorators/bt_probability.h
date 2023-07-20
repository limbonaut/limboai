/* bt_probability.h */

#ifndef BT_PROBABILITY_H
#define BT_PROBABILITY_H

#include "bt_decorator.h"

#include "core/object/object.h"

class BTProbability : public BTDecorator {
	GDCLASS(BTProbability, BTDecorator);

private:
	float run_chance = 0.5;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual int _tick(double p_delta) override;

public:
	void set_run_chance(float p_value) {
		run_chance = p_value;
		emit_changed();
	}
	float get_run_chance() const { return run_chance; }
};

#endif // BT_PROBABILITY_H