/**
 * bb_node.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bb_node.h"

Variant BBNode::get_value(Node *p_scene_root, const Ref<Blackboard> &p_blackboard, const Variant &p_default) {
	ERR_FAIL_NULL_V_MSG(p_scene_root, Variant(), "BBNode: get_value() failed - scene_root is null.");
	ERR_FAIL_COND_V_MSG(p_blackboard.is_null(), Variant(), "BBNode: get_value() failed - blackboard is null.");

	Variant val;
	if (get_value_source() == SAVED_VALUE) {
		val = get_saved_value();
	} else {
		val = p_blackboard->get_var(get_variable(), p_default);
	}

	if (val.get_type() == Variant::NODE_PATH) {
		return p_scene_root->get_node_or_null(val);
	} else if (val.get_type() == Variant::OBJECT || val.get_type() == Variant::NIL) {
		return val;
	} else {
		WARN_PRINT("BBNode: Unexpected variant type: " + Variant::get_type_name(val.get_type()) + ". Returning default value.");
		return p_default;
	}
}
