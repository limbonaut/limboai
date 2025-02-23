/**
 * bt_composite.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_composite.h"

PackedStringArray BTComposite::get_configuration_warnings() {
	PackedStringArray warnings = BTTask::get_configuration_warnings();
	if (get_enabled_child_count() < 1) {
		warnings.append("Composite should have at least one child task.");
	}
	return warnings;
}
