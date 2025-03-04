/**
 * edscale.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef COMPAT_EDSCALE_H
#define COMPAT_EDSCALE_H

#ifdef LIMBOAI_MODULE
#include "editor/themes/editor_scale.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
using namespace godot;
#define EDSCALE (EditorInterface::get_singleton()->get_editor_scale())
#endif // LIMBOAI_GDEXTENSION

#endif // COMPAT_EDSCALE_H
