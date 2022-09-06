/* bt_console_print.h */

#ifndef BT_CONSOLE_PRINT_H
#define BT_CONSOLE_PRINT_H

#include "bt_action.h"
#include "core/object.h"
#include "core/variant.h"

class BTConsolePrint : public BTAction {
	GDCLASS(BTConsolePrint, BTAction);

private:
	String text;
	PoolStringArray format_var_args;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual int _tick(float p_delta);

public:
	void set_text(String p_value) {
		text = p_value;
		emit_changed();
	}
	String get_text() const { return text; }

	void set_format_var_args(const PoolStringArray &p_value) {
		format_var_args = p_value;
		emit_changed();
	}
	PoolStringArray get_format_var_args() const { return format_var_args; }

	virtual String get_configuration_warning() const;
};

#endif // BT_CONSOLE_PRINT_H