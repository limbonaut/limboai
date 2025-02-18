/**
 * forward_decl.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef FORWARD_DECL_H
#define FORWARD_DECL_H

// * It's necessary to enclose Godot forward declarations within the `godot`
// * namespace in GDExtension.

#ifdef LIMBOAI_MODULE
#define GODOT_FORWARD_DECLARATIONS()
#define ENDOF_FORWARD_DECLARATIONS()
#endif // ! LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#define GODOT_FORWARD_DECLARATIONS() namespace godot {
#define ENDOF_FORWARD_DECLARATIONS() } //namespace godot
#endif // ! LIMBOAI_GDEXTENSION

#endif // ! FORWARD_DECL_H
