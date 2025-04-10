/**
 * limbo_compat.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBO_COMPAT_H
#define LIMBO_COMPAT_H

/**
 *	Macros and definitions to bridge differences between GDExtension and Godot APIs.
 *  This helps us writing compatible code with both module and GDExtension.
 *  Additional component-specific helpers can be found in the "compat/" directory.
 */

#ifdef LIMBOAI_MODULE

// *** API abstractions: Module edition

#define RESOURCE_SAVE(m_res, m_path, m_flags) ResourceSaver::save(m_res, m_path, m_flags)
#define FILE_EXISTS(m_path) FileAccess::exists(m_path)
#define DIR_ACCESS_CREATE() DirAccess::create(DirAccess::ACCESS_RESOURCES)
#define ADD_STYLEBOX_OVERRIDE(m_control, m_name, m_stylebox) (m_control->add_theme_style_override(m_name, m_stylebox))

// * Enum

#define LW_KEY(key) (Key::key)
#define LW_KEY_MASK(mask) (KeyModifierMask::mask)
#define LW_MBTN(key) (MouseButton::key)

#endif // ! LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION

// *** API abstractions: GDExtension edition

#define RESOURCE_SAVE(m_res, m_path, m_flags) ResourceSaver::get_singleton()->save(m_res, m_path, m_flags)
#define FILE_EXISTS(m_path) FileAccess::file_exists(m_path)
#define DIR_ACCESS_CREATE() DirAccess::open("res://")
#define ADD_STYLEBOX_OVERRIDE(m_control, m_name, m_stylebox) (m_control->add_theme_stylebox_override(m_name, m_stylebox))

// * Enum

#define LW_KEY(key) (Key::KEY_##key)
#define LW_KEY_MASK(mask) (KeyModifierMask::KEY_MASK_##mask)
#define LW_MBTN(key) (MouseButton::MOUSE_BUTTON_##key)

#endif // ! LIMBOAI_GDEXTENSION

#endif // LIMBO_COMPAT_H
