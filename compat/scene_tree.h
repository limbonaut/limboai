/**
 * scene_tree.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#ifndef COMPAT_SCENE_TREE_H
#define COMPAT_SCENE_TREE_H

#ifdef LIMBOAI_MODULE
#include "scene/main/scene_tree.h"
#define SCENE_TREE() (SceneTree::get_singleton())
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#define SCENE_TREE() ((godot::SceneTree *)(godot::Engine::get_singleton()->get_main_loop()))
#endif // LIMBOAI_GDEXTENSION

#endif // COMPAT_SCENE_TREE_H
