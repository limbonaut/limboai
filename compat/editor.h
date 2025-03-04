/**
 * editor.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#ifndef COMPAT_EDITOR_H
#define COMPAT_EDITOR_H

#ifdef TOOLS_ENABLED

#ifdef LIMBOAI_MODULE
#include <editor/editor_file_system.h>
#include <editor/editor_interface.h>
#define EDITOR_FILE_SYSTEM() (EditorFileSystem::get_singleton())
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/editor_file_system.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
using namespace godot;
#define EDITOR_FILE_SYSTEM() (EditorInterface::get_singleton()->get_resource_filesystem())
#endif // LIMBOAI_GDEXTENSION

// Shared.
void SHOW_BUILTIN_DOC(const String &p_topic);
void EDIT_SCRIPT(const String &p_path);

#endif // TOOLS_ENABLED

#endif // COMPAT_EDITOR_H
