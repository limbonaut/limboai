/**
 * editor_settings.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "editor_settings.h"

#ifdef LIMBOAI_GDEXTENSION

Variant _EDITOR_GET(const String &p_setting) {
	Ref<EditorSettings> es = EditorInterface::get_singleton()->get_editor_settings();
	ERR_FAIL_COND_V_MSG(es.is_null() || !es->has_setting(p_setting), Variant(), "Couldn't get editor setting: " + p_setting);
	return es->get(p_setting);
}

#endif // LIMBOAI_GDEXTENSION
