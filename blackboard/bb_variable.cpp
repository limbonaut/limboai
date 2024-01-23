/**
 * bb_variable.cpp
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bb_variable.h"

void BBVariable::unref() {
	if (data && data->refcount.unref()) {
		memdelete(data);
	}
	data = nullptr;
}

// void BBVariable::init_ref() {
// 	if (data) {
// 		unref();
// 	}
// 	data = memnew(Data);
// 	data->refcount.init();
// }

void BBVariable::set_value(const Variant &p_value) {
	data->value = p_value;
}

Variant BBVariable::get_value() const {
	return data->value;
}

void BBVariable::set_type(Variant::Type p_type) {
	data->type = p_type;
}

Variant::Type BBVariable::get_type() const {
	return data->type;
}

void BBVariable::set_hint(PropertyHint p_hint) {
	data->hint = p_hint;
}

PropertyHint BBVariable::get_hint() const {
	return data->hint;
}

void BBVariable::set_hint_string(const String &p_hint_string) {
	data->hint_string = p_hint_string;
}

String BBVariable::get_hint_string() const {
	return data->hint_string;
}

BBVariable BBVariable::duplicate() const {
	BBVariable var;
	var.data->hint = data->hint;
	var.data->hint_string = data->hint_string;
	var.data->type = data->type;
	var.data->value = data->value;
	return var;
}

bool BBVariable::is_same_prop_info(const BBVariable &p_other) const {
	if (data->type != p_other.data->type) {
		return false;
	}
	if (data->hint != p_other.data->hint) {
		return false;
	}
	if (data->hint_string != p_other.data->hint_string) {
		return false;
	}
	return true;
}

void BBVariable::copy_prop_info(const BBVariable &p_other) {
	data->type = p_other.data->type;
	data->hint = p_other.data->hint;
	data->hint_string = p_other.data->hint_string;
}

bool BBVariable::operator==(const BBVariable &p_var) const {
	if (data == p_var.data) {
		return true;
	}

	if (!data || !p_var.data) {
		return false;
	}

	if (data->type != p_var.data->type) {
		return false;
	}

	if (data->hint != p_var.data->hint) {
		return false;
	}

	if (data->value != p_var.data->value) {
		return false;
	}

	if (data->hint_string != p_var.data->hint_string) {
		return false;
	}

	return true;
}

bool BBVariable::operator!=(const BBVariable &p_var) const {
	return !(*this == p_var);
}

void BBVariable::operator=(const BBVariable &p_var) {
	if (this == &p_var) {
		return;
	}

	unref();

	if (p_var.data && p_var.data->refcount.ref()) {
		data = p_var.data;
	}
}

BBVariable::BBVariable(const BBVariable &p_var) {
	if (p_var.data && p_var.data->refcount.ref()) {
		data = p_var.data;
	}
}

BBVariable::BBVariable() {
	data = memnew(Data);
	data->refcount.init();
}

BBVariable::~BBVariable() {
	unref();
}
