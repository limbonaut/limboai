#/**
 * limbo_def.h
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBO_DEF_H
#define LIMBO_DEF_H

#ifdef LIMBOAI_MODULE

#include "core/string/print_string.h"

#define IS_CLASS(m_obj, m_class) (m_obj->is_class_ptr(m_class::get_class_ptr_static()))
#define RAND_RANGE(m_from, m_to) (Math::random(m_from, m_to))
#define RANDF() (Math::randf())
#define PRINT_LINE(...) (print_line(__VA_ARGS__))
#define IS_DEBUGGER_ACTIVE() (EngineDebugger::is_active())
#define GET_SCENE_TREE() (SceneTree::get_singleton())
#define VCALL(m_method) (GDVIRTUAL_CALL(method))
#define VCALL_ARGS(method, ...) (call(LSNAME(method), __VA_ARGS__))

#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

#define IS_CLASS(m_obj, m_class) (m_obj->get_class_static() == m_class::get_class_static())
#define RAND_RANGE(m_from, m_to) (UtilityFunctions::randf_range(m_from, m_to))
#define RANDF() (UtilityFunctions::randf())
#define PRINT_LINE(...) (UtilityFunctions::print(__VA_ARGS__))
#define IS_DEBUGGER_ACTIVE() (EngineDebugger::get_singleton()->is_active())
#define GET_SCENE_TREE() ((SceneTree *)(Engine::get_singleton()->get_main_loop()))
#define VCALL(m_name) (call(LSNAME(m_name)))
#define VCALL_ARGS(m_name, ...) (call(LSNAME(m_name), __VA_ARGS__))

#define EDITOR_GET(m_var) _EDITOR_GET(m_var)
Variant _EDITOR_GET(const String &p_setting);

#define EDSCALE ((int)EDITOR_GET("interface/editor/display_scale"))

String TTR(const String &p_text, const String &p_context = "");

#endif // LIMBOAI_GDEXTENSION

#endif // LIMBO_DEF_H
