/**
 * register_types.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBOAI_REGISTER_TYPES_H
#define LIMBOAI_REGISTER_TYPES_H

#include "modules/register_module_types.h"

void initialize_limboai_module(ModuleInitializationLevel p_level);
void uninitialize_limboai_module(ModuleInitializationLevel p_level);

#endif // LIMBOAI_REGISTER_TYPES_H
