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
#include "core/variant/binder_common.h"
#include "core/variant/variant.h"
#include "scene/resources/texture.h"

#define LOGICAL_XOR(a, b) (a) ? !(b) : (b)

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

	enum Operation {
		OP_NONE,
		OP_ADDITION,
		OP_SUBTRACTION,
		OP_MULTIPLICATION,
		OP_DIVISION,
		OP_MODULO,
		OP_POWER,
		OP_BIT_SHIFT_LEFT,
		OP_BIT_SHIFT_RIGHT,
		OP_BIT_AND,
		OP_BIT_OR,
		OP_BIT_XOR,
	};

protected:
	static LimboUtility *singleton;
	static void _bind_methods();

public:
	static LimboUtility *get_singleton();

	String decorate_var(String p_variable) const;
	String get_status_name(int p_status) const;
	Ref<Texture2D> get_task_icon(String p_class_or_script_path) const;

	String get_check_operator_string(CheckType p_check_type) const;
	bool perform_check(CheckType p_check_type, const Variant &left_value, const Variant &right_value);

	String get_operation_string(Operation p_operation) const;
	Variant perform_operation(Operation p_operation, const Variant &left_value, const Variant &right_value);

	LimboUtility();
	~LimboUtility();
};

VARIANT_ENUM_CAST(LimboUtility::CheckType);
VARIANT_ENUM_CAST(LimboUtility::Operation);

#endif // LIMBO_UTILITY_H