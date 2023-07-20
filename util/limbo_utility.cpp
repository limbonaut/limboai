/* limbo_utility.cpp */

#include "limbo_utility.h"

#include "modules/limboai/bt/bt_task.h"

#include "core/variant/variant.h"
#include "scene/resources/texture.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#endif // TOOLS_ENABLED

LimboUtility *LimboUtility::singleton = nullptr;

LimboUtility *LimboUtility::get_singleton() {
	return singleton;
}

String LimboUtility::decorate_var(String p_variable) const {
	String var = p_variable.trim_prefix("$").trim_prefix("\"").trim_suffix("\"");
	if (var.find(" ") == -1 and not var.is_empty()) {
		return vformat("$%s", var);
	} else {
		return vformat("$\"%s\"", var);
	}
}

String LimboUtility::get_status_name(int p_status) const {
	switch (p_status) {
		case BTTask::FRESH:
			return "FRESH";
		case BTTask::RUNNING:
			return "RUNNING";
		case BTTask::FAILURE:
			return "FAILURE";
		case BTTask::SUCCESS:
			return "SUCCESS";
		default:
			return "";
	}
}

Ref<Texture2D> LimboUtility::get_task_icon(String p_class_or_script_path) const {
#ifdef TOOLS_ENABLED
	ERR_FAIL_COND_V_MSG(p_class_or_script_path.is_empty(), Variant(), "BTTask: script path or class cannot be empty.");

	if (p_class_or_script_path.begins_with("res:")) {
		Ref<Script> s = ResourceLoader::load(p_class_or_script_path, "Script");
		return EditorNode::get_singleton()->get_object_icon(s.ptr(), "BTTask");
	}
	// TODO: Walk inheritance tree until icon is found.
	return EditorNode::get_singleton()->get_class_icon(p_class_or_script_path, "BTTask");
#endif // TOOLS_ENABLED
	// Note: class icons are not available at runtime as they are part of the editor theme.
	return nullptr;
}

void LimboUtility::_bind_methods() {
	ClassDB::bind_method(D_METHOD("decorate_var", "p_variable"), &LimboUtility::decorate_var);
	ClassDB::bind_method(D_METHOD("get_status_name", "p_status"), &LimboUtility::get_status_name);
	ClassDB::bind_method(D_METHOD("get_task_icon"), &LimboUtility::get_task_icon);
}

LimboUtility::LimboUtility() {
	singleton = this;
}

LimboUtility::~LimboUtility() {
	singleton = nullptr;
}
