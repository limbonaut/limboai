/* bt_for_each.h */

#ifndef BT_FOR_EACH_H
#define BT_FOR_EACH_H

#include "bt_decorator.h"
#include "core/object/object.h"

class BTForEach : public BTDecorator {
	GDCLASS(BTForEach, BTDecorator);

private:
	String array_var;
	String save_var;

	int current_idx;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual void _enter() override;
	virtual int _tick(double p_delta) override;

public:
	void set_array_var(String p_value) {
		array_var = p_value;
		emit_changed();
	}
	String get_array_var() const { return array_var; }
	void set_save_var(String p_value) {
		save_var = p_value;
		emit_changed();
	}
	String get_save_var() const { return save_var; }
};

#endif // BT_FOR_EACH_H