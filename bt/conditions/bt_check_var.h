/**
 * bt_check_var.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_CHECK_VAR_H
#define BT_CHECK_VAR_H

#include "bt_condition.h"

#include "modules/limboai/blackboard/bb_param/bb_variant.h"

#include "core/object/class_db.h"
#include "core/object/object.h"

class BTCheckVar : public BTCondition {
	GDCLASS(BTCheckVar, BTCondition);

public:
	enum CheckType : unsigned int {
		CHECK_EQUAL,
		CHECK_LESS_THAN,
		CHECK_LESS_THAN_OR_EQUAL,
		CHECK_GREATER_THAN,
		CHECK_GREATER_THAN_OR_EQUAL,
		CHECK_NOT_EQUAL
	};

private:
	String variable;
	CheckType check_type = CheckType::CHECK_EQUAL;
	Ref<BBVariant> value;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual int _tick(double p_delta) override;

public:
	virtual String get_configuration_warning() const override;

	void set_variable(String p_variable);
	String get_variable() const { return variable; }

	void set_check_type(CheckType p_check_type);
	CheckType get_check_type() const { return check_type; }

	void set_value(Ref<BBVariant> p_value);
	Ref<BBVariant> get_value() const { return value; }
};

#endif // BT_CHECK_VAR_H