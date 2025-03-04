/**
 * project_settings.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef COMPAT_PROJECT_SETTINGS_H
#define COMPAT_PROJECT_SETTINGS_H

#ifdef LIMBOAI_MODULE

#include "core/config/project_settings.h"

#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/variant.hpp>
using namespace godot;

#define GLOBAL_GET(m_var) ProjectSettings::get_singleton()->get_setting_with_override(m_var)

#define GLOBAL_DEF(m_var, m_value) _GLOBAL_DEF(m_var, m_value)
Variant _GLOBAL_DEF(const String &p_var, const Variant &p_default, bool p_restart_if_changed = false, bool p_ignore_value_in_docs = false, bool p_basic = false, bool p_internal = false);
Variant _GLOBAL_DEF(const PropertyInfo &p_info, const Variant &p_default, bool p_restart_if_changed = false, bool p_ignore_value_in_docs = false, bool p_basic = false, bool p_internal = false);

#endif // LIMBOAI_GDEXTENSION

#endif // COMPAT_PROJECT_SETTINGS_H
