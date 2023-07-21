/**
 * bb_variant.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bb_variant.h"
#include "core/object/object.h"
#include "core/variant/variant.h"

void BBVariant::set_type(Variant::Type p_type) {
	type = p_type;
	notify_property_list_changed();
	emit_changed();
}

void BBVariant::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_type", "p_type"), &BBVariant::set_type);

	String vtypes;
	for (int i = 0; i < Variant::VARIANT_MAX; i++) {
		if (i > 0) {
			vtypes += ",";
		}
		vtypes += Variant::get_type_name(Variant::Type(i));
	}
	ADD_PROPERTY(PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM, vtypes), "set_type", "get_type");
}

BBVariant::BBVariant() {
	type = Variant::NIL;
}
