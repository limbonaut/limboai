/* bt_console_print.cpp */

#include "bt_console_print.h"
#include "core/object/object.h"
#include "core/string/print_string.h"
#include "modules/limboai/bt/actions/bt_action.h"

String BTConsolePrint::_generate_name() const {
	String tx = text;
	if (text.length() > 30) {
		tx = text.substr(0, 30) + "...";
	}
	tx = tx.replace("\"", "\\\"");
	if (format_var_args.size() > 0) {
		return vformat("ConsolePrint  text: \"%s\"  format_args: %s", tx, format_var_args);
	}
	return vformat("ConsolePrint \"%s\"", tx);
}

int BTConsolePrint::_tick(float p_delta) {
	switch (format_var_args.size()) {
		case 0: {
			print_line(text);
		} break;
		case 1: {
			print_line(vformat(text, get_blackboard()->get_var(format_var_args[0], "")));
		} break;
		case 2: {
			print_line(vformat(text, get_blackboard()->get_var(format_var_args[0], ""),
					get_blackboard()->get_var(format_var_args[1], "")));
		} break;
		case 3: {
			print_line(vformat(text, get_blackboard()->get_var(format_var_args[0], ""),
					get_blackboard()->get_var(format_var_args[1], ""),
					get_blackboard()->get_var(format_var_args[2], "")));
		} break;
		case 4: {
			print_line(vformat(text, get_blackboard()->get_var(format_var_args[0], ""),
					get_blackboard()->get_var(format_var_args[1], ""),
					get_blackboard()->get_var(format_var_args[2], ""),
					get_blackboard()->get_var(format_var_args[3], "")));
		} break;
		case 5:
		default: {
			print_line(vformat(text, get_blackboard()->get_var(format_var_args[0], ""),
					get_blackboard()->get_var(format_var_args[1], ""),
					get_blackboard()->get_var(format_var_args[2], ""),
					get_blackboard()->get_var(format_var_args[3], ""),
					get_blackboard()->get_var(format_var_args[4], "")));
		} break;
	}
	return SUCCESS;
}

String BTConsolePrint::get_configuration_warning() const {
	String warning = BTAction::get_configuration_warning();
	if (!warning.is_empty()) {
		warning += "\n";
	}
	if (format_var_args.size() > 5) {
		warning += "ConsolePrint supports up to 5 format arguments.\n";
	}
	return warning;
}

void BTConsolePrint::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_text", "p_text"), &BTConsolePrint::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &BTConsolePrint::get_text);
	ClassDB::bind_method(D_METHOD("set_format_var_args", "p_variables"), &BTConsolePrint::set_format_var_args);
	ClassDB::bind_method(D_METHOD("get_format_var_args"), &BTConsolePrint::get_format_var_args);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "format_var_args"), "set_format_var_args", "get_format_var_args");
}