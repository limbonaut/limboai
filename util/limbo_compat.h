#/**
 * limbo_compat.h
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

/*
 *	Defines and funcs that help to bridge some differences between GDExtension and Godot APIs.
 *  This helps us writing compatible code with both module and GDExtension.
 */

#ifndef LIMBO_COMPAT_H
#define LIMBO_COMPAT_H

#ifdef LIMBOAI_MODULE

#include "core/string/print_string.h"

// * API abstractions: Module edition

#define SCRIPT_EDITOR() (ScriptEditor::get_singleton())
#define EDITOR_FILE_SYSTEM() (EditorFileSystem::get_singleton())
#define EDITOR_SETTINGS() (EditorSettings::get_singleton())
#define BASE_CONTROL() (EditorNode::get_singleton()->get_gui_base())
#define MAIN_SCREEN_CONTROL(EditorNode::get_singleton()->get_main_screen_control())
#define SCENE_TREE() (SceneTree::get_singleton())
#define IS_DEBUGGER_ACTIVE() (EngineDebugger::is_active())

#define PRINT_LINE(...) (print_line(__VA_ARGS__))
#define IS_CLASS(m_obj, m_class) (m_obj->is_class_ptr(m_class::get_class_ptr_static()))
#define RAND_RANGE(m_from, m_to) (Math::random(m_from, m_to))
#define RANDF() (Math::randf())
#define VCALL(m_method) (GDVIRTUAL_CALL(method))
#define VCALL_ARGS(method, ...) (call(LW_NAME(method), __VA_ARGS__))
#define BUTTON_SET_ICON(m_btn, m_icon) m_btn->set_icon(m_icon)
#define RESOURCE_LOAD(m_path, m_hint) ResourceLoader::load(m_path, m_hint)
#define RESOURCE_LOAD_NO_CACHE(m_path, m_hint) ResourceLoader::load(m_path, m_hint, ResourceLoader::CACHE_MODE_IGNORE)
#define RESOURCE_SAVE(m_res, m_path, m_flags) ResourceSaver::save(m_res, m_path, m_flags)
#define RESOURCE_IS_CACHED(m_path) (ResourceCache::has(m_path))
#define RESOURCE_GET_TYPE(m_path) (ResourceLoader::get_resource_type(m_path))
#define RESOURCE_EXISTS(m_path, m_type_hint) (ResourceLoader::exists(m_path, m_type_hint))
#define GET_PROJECT_SETTINGS_DIR() EditorPaths::get_singleton()->get_project_settings_dir()
#define EDIT_RESOURCE(m_res) EditorNode::get_singleton()->edit_resource(m_res)
#define INSPECTOR_GET_EDITED_OBJECT() (InspectorDock::get_inspector_singleton()->get_edited_object())
#define SET_MAIN_SCREEN_EDITOR(m_name) (EditorNode::get_singleton()->select_editor_by_name(m_name))
#define FILE_EXISTS(m_path) FileAccess::exists(m_path)
#define DIR_ACCESS_CREATE() DirAccess::create(DirAccess::ACCESS_RESOURCES)

#define SHOW_DOC(m_doc) (                                \
		ScriptEditor::get_singleton()->goto_help(m_doc); \
		EditorNode::get_singleton()->set_visible_editor(EditorNode::EDITOR_SCRIPT);)

#define VARIANT_EVALUATE(m_op, m_lvalue, m_rvalue, r_ret) r_ret = Variant::evaluate(m_op, m_lvalue, m_rvalue)

// * Enum

#define LW_KEY(key) (Key::key)
#define LW_KEY_MASK(mask) (KeyModifierMask::##mask)
#define LW_MBTN(key) (MouseButton::key)

#endif // ! LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

// * API abstractions: GDExtension edition

#define SCRIPT_EDITOR() (EditorInterface::get_singleton()->get_script_editor())
#define EDITOR_FILE_SYSTEM() (EditorInterface::get_singleton()->get_resource_filesystem())
#define EDITOR_SETTINGS() (EditorInterface::get_singleton()->get_editor_settings())
#define BASE_CONTROL() (EditorInterface::get_singleton()->get_base_control())
#define MAIN_SCREEN_CONTROL() (EditorInterface::get_singleton()->get_editor_main_screen())
#define SCENE_TREE() ((SceneTree *)(Engine::get_singleton()->get_main_loop()))
#define IS_DEBUGGER_ACTIVE() (EngineDebugger::get_singleton()->is_active())

#define PRINT_LINE(...) (UtilityFunctions::print(__VA_ARGS__))
#define IS_CLASS(m_obj, m_class) (m_obj->get_class_static() == m_class::get_class_static())
#define RAND_RANGE(m_from, m_to) (UtilityFunctions::randf_range(m_from, m_to))
#define RANDF() (UtilityFunctions::randf())
#define VCALL(m_name) (call(LW_NAME(m_name)))
#define VCALL_ARGS(m_name, ...) (call(LW_NAME(m_name), __VA_ARGS__))
#define BUTTON_SET_ICON(m_btn, m_icon) m_btn->set_button_icon(m_icon)
#define RESOURCE_LOAD(m_path, m_hint) ResourceLoader::get_singleton()->load(m_path, m_hint)
#define RESOURCE_LOAD_NO_CACHE(m_path, m_hint) ResourceLoader::get_singleton()->load(m_path, m_hint, ResourceLoader::CACHE_MODE_IGNORE)
#define RESOURCE_SAVE(m_res, m_path, m_flags) ResourceSaver::get_singleton()->save(m_res, m_path, m_flags)
#define RESOURCE_IS_CACHED(m_path) (ResourceLoader::get_singleton()->has_cached(res_path))
#define RESOURCE_GET_TYPE(m_path) (ResourceLoader::get_resource_type(m_path))
#define RESOURCE_EXISTS(m_path, m_type_hint) (ResourceLoader::get_singleton()->exists(m_path, m_type_hint))
#define GET_PROJECT_SETTINGS_DIR() EditorInterface::get_singleton()->get_editor_paths()->get_project_settings_dir()
#define EDIT_RESOURCE(m_res) EditorInterface::get_singleton()->edit_resource(m_res)
void EDIT_SCRIPT(const String &p_path); // TODO: need a module version!
#define INSPECTOR_GET_EDITED_OBJECT() (EditorInterface::get_singleton()->get_inspector()->get_edited_object())
#define SET_MAIN_SCREEN_EDITOR(m_name) (EditorInterface::get_singleton()->set_main_screen_editor(m_name))
#define FILE_EXISTS(m_path) FileAccess::file_exists(m_path)
#define DIR_ACCESS_CREATE() DirAccess::open("res://")

#define SHOW_DOC(m_doc) EditorInterface::get_singleton()->get_script_editor()->get_current_editor()->emit_signal("go_to_help", m_doc)

#define VARIANT_EVALUATE(m_op, m_lvalue, m_rvalue, r_ret)            \
	{                                                                \
		bool r_valid;                                                \
		Variant::evaluate(m_op, m_lvalue, m_rvalue, r_ret, r_valid); \
	}

// * Enums

#define LW_KEY(key) (Key::KEY_##key)
#define LW_KEY_MASK(mask) (KeyModifierMask::KEY_MASK_##mask)
#define LW_MBTN(key) (MouseButton::MOUSE_BUTTON_##key)

// * Missing defines

#define EDITOR_GET(m_var) _EDITOR_GET(m_var)
Variant _EDITOR_GET(const String &p_setting);

#define GLOBAL_GET(m_var) ProjectSettings::get_singleton()->get_setting_with_override(m_var)

#define GLOBAL_DEF(m_var, m_value) _GLOBAL_DEF(m_var, m_value)
Variant _GLOBAL_DEF(const String &p_var, const Variant &p_default, bool p_restart_if_changed = false, bool p_ignore_value_in_docs = false, bool p_basic = false, bool p_internal = false);
Variant _GLOBAL_DEF(const PropertyInfo &p_info, const Variant &p_default, bool p_restart_if_changed = false, bool p_ignore_value_in_docs = false, bool p_basic = false, bool p_internal = false);

#define EDSCALE (EditorInterface::get_singleton()->get_editor_scale())

String TTR(const String &p_text, const String &p_context = "");

#endif // ! LIMBOAI_GDEXTENSION

// * Shared defines

#define VARIANT_IS_ARRAY(m_variant) (m_variant.get_type() >= Variant::ARRAY)
#define VARIANT_IS_NUM(m_variant) (m_variant.get_type() == Variant::INT || m_variant.get_type() == Variant::FLOAT)

inline void VARIANT_DELETE_IF_OBJECT(Variant m_variant) {
	if (m_variant.get_type() == Variant::OBJECT) {
		Ref<RefCounted> r = m_variant;
		if (r.is_null()) {
			memdelete((Object *)m_variant);
		}
	}
}

#endif // LIMBO_COMPAT_H
