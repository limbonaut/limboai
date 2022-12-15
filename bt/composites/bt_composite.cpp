/* bt_composite.cpp */

#include "bt_composite.h"

String BTComposite::get_configuration_warning() const {
	String warning = BTTask::get_configuration_warning();
	if (!warning.is_empty()) {
		warning += "\n";
	}
	if (get_child_count() < 1) {
		warning += "Composite should have at least one child task.\n";
	}
	return warning;
}