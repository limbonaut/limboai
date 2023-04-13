/* limbo_utility.cpp */

#include "limbo_utility.h"
#include "bt/bt_task.h"
#include "core/io/image_loader.h"
#include "core/variant/variant.h"
#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#include "scene/resources/texture.h"

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
	ERR_FAIL_COND_V_MSG(p_class_or_script_path.is_empty(), Variant(), "BTTask: script path or class cannot be empty.");

	String base_type = p_class_or_script_path;
	if (p_class_or_script_path.begins_with("res:")) {
		Ref<Script> script = ResourceLoader::load(p_class_or_script_path, "Script");
		Ref<Script> base_script = script;
		while (base_script.is_valid()) {
			StringName name = EditorNode::get_editor_data().script_class_get_name(base_script->get_path());
			String icon_path = EditorNode::get_editor_data().script_class_get_icon_path(name);
			if (!icon_path.is_empty()) {
				Ref<Image> img = memnew(Image);
				Error err = ImageLoader::load_image(icon_path, img);
				if (err == OK) {
					Ref<ImageTexture> icon = memnew(ImageTexture);
					img->resize(16 * EDSCALE, 16 * EDSCALE, Image::INTERPOLATE_LANCZOS);
					icon->create_from_image(img);
					return icon;
				}
			}
			base_script = base_script->get_base_script();
		}
		base_type = script->get_instance_base_type();
	}

	// TODO: Walk inheritance tree until icon is found.
	return EditorNode::get_singleton()->get_class_icon(base_type, "BTTask");
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
