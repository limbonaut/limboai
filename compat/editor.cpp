/**
 * editor.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#include "editor.h"

#ifdef TOOLS_ENABLED

#include "resource_loader.h"

#ifdef LIMBOAI_MODULE
#include "editor/editor_interface.h"
#include "editor/script/script_editor_plugin.h"
#elif LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/script_editor.hpp>
#endif

void SHOW_BUILTIN_DOC(const String &p_topic) {
	EditorInterface::get_singleton()->get_script_editor()->goto_help(p_topic);
	EditorInterface::get_singleton()->set_main_screen_editor("Script");
}

void EDIT_SCRIPT(const String &p_path) {
	Ref<Resource> res = RESOURCE_LOAD(p_path, "Script");
	ERR_FAIL_COND_MSG(res.is_null(), "Failed to load script: " + p_path);
	EditorInterface::get_singleton()->edit_resource(res);
}

#endif // ! TOOLS_ENABLED
