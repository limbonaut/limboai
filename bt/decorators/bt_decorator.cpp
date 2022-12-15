/* bt_decorator.cpp */

#include "bt_decorator.h"

String BTDecorator::get_configuration_warning() const {
	String warning = BTTask::get_configuration_warning();
	if (!warning.is_empty()) {
		warning += "\n";
	}
	if (get_child_count() != 1) {
		warning += "Decorator should have a single child task.\n";
	}
	return warning;
}
