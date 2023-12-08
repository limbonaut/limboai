/**
 * bb_param.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_PARAM_H
#define BB_PARAM_H

#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/util/limbo_utility.h"

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/typedefs.h"
#include "core/variant/variant.h"

class BBParam : public Resource {
	GDCLASS(BBParam, Resource);

public:
	enum ValueSource : unsigned int {
		SAVED_VALUE,
		BLACKBOARD_VAR
	};

private:
	ValueSource value_source;
	Variant saved_value;
	String variable;

	_FORCE_INLINE_ void _update_name() {
		set_name((value_source == SAVED_VALUE) ? String(saved_value) : LimboUtility::get_singleton()->decorate_var(variable));
	}

protected:
	static void _bind_methods();

	_FORCE_INLINE_ void _assign_default_value() {
		Callable::CallError err;
		Variant::construct(get_type(), saved_value, nullptr, 0, err);
	}

	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	virtual Variant::Type get_type() const { return Variant::NIL; }

	void set_value_source(ValueSource p_value);
	ValueSource get_value_source() const { return value_source; }

	void set_saved_value(Variant p_value);
	Variant get_saved_value();

	void set_variable(const String &p_value);
	String get_variable() const { return variable; }

	virtual String to_string() override;

	virtual Variant get_value(Object *p_agent, const Ref<Blackboard> &p_blackboard, const Variant &p_default = Variant());

	BBParam();
};

#endif // BB_PARAM_H