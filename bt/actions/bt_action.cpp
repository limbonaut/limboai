/* bt_action.cpp */

#include "bt_action.h"

String BTAction::get_configuration_warning() const {
	String warning = BTTask::get_configuration_warning();
	if (!warning.is_empty()) {
		warning += "\n";
	}
	if (get_child_count() != 0) {
		warning += "Action can't have child tasks.\n";
	}
	return warning;
}