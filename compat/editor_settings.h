/**
 * editor_settings.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef COMPAT_EDITOR_SETTINGS_H
#define COMPAT_EDITOR_SETTINGS_H

#ifdef LIMBOAI_MODULE

#include "editor/editor_settings.h"

#define EDITOR_SETTINGS() (EditorSettings::get_singleton())

#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
using namespace godot;

#define EDITOR_SETTINGS() (EditorInterface::get_singleton()->get_editor_settings())

#define EDITOR_GET(m_var) _EDITOR_GET(m_var)
Variant _EDITOR_GET(const String &p_setting);

#define EDITOR_DEF(m_setting, m_value)                                                                         \
	do { /* do-while(0) ideom to avoid any potential semicolon errors. */                                      \
		EditorInterface::get_singleton()->get_editor_settings()->set_initial_value(m_setting, m_value, false); \
		if (!EDITOR_SETTINGS()->has_setting(m_setting)) {                                                      \
			EDITOR_SETTINGS()->set_setting(m_setting, m_value);                                                \
		}                                                                                                      \
	} while (0)

#endif // LIMBOAI_GDEXTENSION

#endif // COMPAT_EDITOR_SETTINGS_H
