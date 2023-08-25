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

#include "../bt_condition.h"

#include "modules/limboai/blackboard/bb_param/bb_variant.h"
#include "modules/limboai/util/limbo_utility.h"

class BTCheckVar : public BTCondition {
	GDCLASS(BTCheckVar, BTCondition);
	TASK_CATEGORY(Conditions);

private:
	String variable;
	LimboUtility::CheckType check_type = LimboUtility::CheckType::CHECK_EQUAL;
	Ref<BBVariant> value;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual int _tick(double p_delta) override;

public:
	virtual PackedStringArray get_configuration_warnings() const override;

	void set_variable(String p_variable);
	String get_variable() const { return variable; }

	void set_check_type(LimboUtility::CheckType p_check_type);
	LimboUtility::CheckType get_check_type() const { return check_type; }

	void set_value(Ref<BBVariant> p_value);
	Ref<BBVariant> get_value() const { return value; }
};

#endif // BT_CHECK_VAR_H