/**
 * bb_variable.h
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_VARIABLE_H
#define BB_VARIABLE_H

#include "core/object/object.h"
#include "core/templates/safe_refcount.h"
#include "core/variant/variant.h"

class BBVariable {
private:
	struct Data {
		SafeRefCount refcount;
		Variant value;
		Variant::Type type = Variant::NIL;
		PropertyHint hint = PropertyHint::PROPERTY_HINT_NONE;
		String hint_string;
		// bool bound = false;
		// uint64_t bound_object = 0;
		// StringName bound_property;
	};

	Data *data = nullptr;
	void unref();
	// void init_ref();

public:
	void set_value(const Variant &p_value);
	Variant get_value() const;

	void set_type(Variant::Type p_type);
	Variant::Type get_type() const;

	void set_hint(PropertyHint p_hint);
	PropertyHint get_hint() const;

	void set_hint_string(const String &p_hint_string);
	String get_hint_string() const;

	BBVariable duplicate() const;

	bool is_same_prop_info(const BBVariable &p_other) const;
	void copy_prop_info(const BBVariable &p_other);

	// bool is_bound() { return bound; }

	// void bind(Node *p_root, NodePath p_path);
	// void unbind();

	bool operator==(const BBVariable &p_var) const;
	bool operator!=(const BBVariable &p_var) const;
	void operator=(const BBVariable &p_var);

	BBVariable(const BBVariable &p_var);
	BBVariable(Variant::Type p_type = Variant::Type::NIL, PropertyHint p_hint = PROPERTY_HINT_NONE, const String &p_hint_string = "");
	~BBVariable();
};

#endif // BB_VARIABLE_H