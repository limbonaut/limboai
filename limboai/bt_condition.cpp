/* bt_condition.cpp */

#include "bt_condition.h"

String BTCondition::get_configuration_warning() const {
	String warning = BTTask::get_configuration_warning();
	if (!warning.empty()) {
		warning += "\n";
	}
	if (get_child_count() != 0) {
		warning += "Condition can't have child tasks.\n";
	}
	return warning;
}