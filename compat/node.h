/**
 * compat/node.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#pragma once

#ifdef LIMBOAI_MODULE

#define IS_NODE_READY(m_node) (m_node->is_ready())

#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION

#define IS_NODE_READY(m_node) (m_node->is_node_ready())

#endif // LIMBOAI_GDEXTENSION
