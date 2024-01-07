/**
 * limbo_def.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "limbo_def.h"

#ifdef LIMBOAI_MODULE

void EDIT_SCRIPT(const String &p_path) {
	Ref<Resource> res = ScriptEditor::get_singleton()->open_file(p_path);
	ERR_FAIL_COND_MSG(res.is_null(), "Failed to load script: " + p_path);
	EditorNode::get_singleton()->edit_resource(res);
}

#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION

#include "godot_cpp/classes/editor_interface.hpp"
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/translation_server.hpp>

using namespace godot;

Variant _EDITOR_GET(const String &p_setting) {
	Ref<EditorSettings> es = EditorInterface::get_singleton()->get_editor_settings();
	ERR_FAIL_COND_V(es.is_null() || !es->has_setting(p_setting), Variant());
	return es->get(p_setting);
}

String TTR(const String &p_text, const String &p_context) {
	if (TranslationServer::get_singleton()) {
		return TranslationServer::get_singleton()->translate(p_text, p_context);
	}

	return p_text;
}

void EDIT_SCRIPT(const String &p_path) {
	Ref<Script> res = RESOURCE_LOAD(p_path, "Script");
	ERR_FAIL_COND_MSG(res.is_null(), "Failed to load script: " + p_path);
	EditorInterface::get_singleton()->edit_script(res);
}

#endif // LIMBOAI_GDEXTENSION
