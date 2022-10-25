/* bb_variant.cpp */

#include "bb_variant.h"
#include "core/object.h"
#include "core/variant.h"

void BBVariant::set_type(Variant::Type p_type) {
	type = p_type;
	property_list_changed_notify();
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
