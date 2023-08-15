/**
 * limbo_utility.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBO_UTILITY_H
#define LIMBO_UTILITY_H

#include "core/object/object.h"

#include "core/object/class_db.h"
#include "core/variant/variant.h"
#include "scene/resources/texture.h"

class LimboUtility : public Object {
	GDCLASS(LimboUtility, Object);

public:
	enum CheckType : unsigned int {
		CHECK_EQUAL,
		CHECK_LESS_THAN,
		CHECK_LESS_THAN_OR_EQUAL,
		CHECK_GREATER_THAN,
		CHECK_GREATER_THAN_OR_EQUAL,
		CHECK_NOT_EQUAL
	};

protected:
	static LimboUtility *singleton;
	static void _bind_methods();

public:
	static LimboUtility *get_singleton();

	String decorate_var(String p_variable) const;
	String get_status_name(int p_status) const;
	Ref<Texture2D> get_task_icon(String p_class_or_script_path) const;

	String get_check_operator_string(CheckType p_check_type);
	bool perform_check(CheckType p_check_type, const Variant &left_value, const Variant &right_value);

	LimboUtility();
	~LimboUtility();
};

VARIANT_ENUM_CAST(LimboUtility::CheckType);

#endif // LIMBO_UTILITY_H