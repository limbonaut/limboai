/**
 * performance.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#ifndef COMPAT_PERFORMANCE_H
#define COMPAT_PERFORMANCE_H

#ifdef LIMBOAI_MODULE
#include "main/performance.h"
#define PERFORMANCE_ADD_CUSTOM_MONITOR(m_id, m_callable) (Performance::get_singleton()->add_custom_monitor(m_id, m_callable, Variant()))
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/performance.hpp>
using namespace godot;
#define PERFORMANCE_ADD_CUSTOM_MONITOR(m_id, m_callable) (Performance::get_singleton()->add_custom_monitor(m_id, m_callable))
#endif // LIMBOAI_GDEXTENSION

#endif // COMPAT_PERFORMANCE_H
