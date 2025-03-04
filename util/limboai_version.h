/**
 * limboai_version.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBOAI_VERSION_H
#define LIMBOAI_VERSION_H

#include "limboai_version.gen.h"

#ifdef LIMBOAI_MODULE
#include "core/string/ustring.h"
#endif

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/variant/string.hpp>
using namespace godot;
#endif

inline String GET_LIMBOAI_VERSION() {
	String version = itos(LIMBOAI_VERSION_MAJOR) + "." + itos(LIMBOAI_VERSION_MINOR);
	if (LIMBOAI_VERSION_PATCH != 0 || strlen(LIMBOAI_VERSION_STATUS) == 0) {
		version += "." + itos(LIMBOAI_VERSION_PATCH);
	}
	if (strlen(LIMBOAI_VERSION_STATUS) > 0) {
		version += "-" + String(LIMBOAI_VERSION_STATUS);
	}
	return version;
}

#define GET_LIMBOAI_FULL_VERSION() GET_LIMBOAI_VERSION() + " [" + LIMBOAI_VERSION_SHORT_HASH + "]"

#endif // LIMBOAI_VERSION_H
