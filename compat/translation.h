/**
 * translation.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#ifndef COMPAT_TRANSLATION_H
#define COMPAT_TRANSLATION_H

#ifdef LIMBOAI_MODULE
#include <core/string/ustring.h>
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/variant/string.hpp>
using namespace godot;
String TTR(const String &p_text, const String &p_context = "");
#define RTR(m_text) TTR(m_text)

#endif // LIMBOAI_GDEXTENSION

#endif // COMPAT_TRANSLATION_H
