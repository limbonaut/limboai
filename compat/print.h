/**
 * print.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#ifndef COMPAT_PRINT_STRING_H
#define COMPAT_PRINT_STRING_H

#ifdef LIMBOAI_MODULE
#include <core/string/print_string.h>
#define PRINT_LINE(...) (print_line(__VA_ARGS__))
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/variant/utility_functions.hpp>
#define PRINT_LINE(...) (godot::UtilityFunctions::print(__VA_ARGS__))
#endif // LIMBOAI_GDEXTENSION

#endif // COMPAT_PRINT_STRING_H
