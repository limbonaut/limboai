/* bt_console_print.h */

#ifndef BT_CONSOLE_PRINT_H
#define BT_CONSOLE_PRINT_H

#include "bt_action.h"
#include "core/object/object.h"
#include "core/variant/variant.h"

class BTConsolePrint : public BTAction {
	GDCLASS(BTConsolePrint, BTAction);

private:
	String text;
	PackedStringArray bb_format_parameters;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual int _tick(float p_delta) override;

public:
	void set_text(String p_value) {
		text = p_value;
		emit_changed();
	}
	String get_text() const { return text; }

	void set_bb_format_parameters(const PackedStringArray &p_value) {
		bb_format_parameters = p_value;
		emit_changed();
	}
	PackedStringArray get_bb_format_parameters() const { return bb_format_parameters; }

	virtual String get_configuration_warning() const override;
};

#endif // BT_CONSOLE_PRINT_H