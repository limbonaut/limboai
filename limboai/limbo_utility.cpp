/* limbo_utility.cpp */

#include "limbo_utility.h"
#include "core/class_db.h"
#include "core/dictionary.h"
#include "core/print_string.h"
#include "core/project_settings.h"
#include "core/variant.h"

String LimboUtility::get_script_class(const Ref<Script> &p_script) {
	String a_name = "";
	Array script_classes = GLOBAL_GET("_global_script_classes");
	String task_filepath = p_script->get_path();
	for (int i = 0; i < script_classes.size(); i++) {
		Dictionary a_class = script_classes[i];
		Variant class_path = a_class["path"];
		// print_line(vformat("Type: %s Path: %s TaskPath: %s", class_path.get_type(), class_path, task_filepath));
		if (a_class["path"] == task_filepath) {
			a_name = a_class["class"];
			break;
		}
	}
	return a_name;
}

void LimboUtility::_bind_methods() {
	// ClassDB::bind_method(D_METHOD("get_class_name", "p_script"), &LimboUtility::get_class_name);
}
