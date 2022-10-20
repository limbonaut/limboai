/* bb_param.h */

#ifndef BB_PARAM_H
#define BB_PARAM_H

#include "core/object.h"
#include "core/resource.h"
#include "core/variant.h"
#include "modules/limboai/blackboard.h"

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

protected:
	static void _bind_methods();

	virtual Variant::Type get_type() const { return Variant::NIL; }

	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	void set_value_source(ValueSource p_value) {
		value_source = p_value;
		property_list_changed_notify();
		emit_changed();
	}
	ValueSource get_value_source() const { return value_source; }

	void set_saved_value(Variant p_value) {
		saved_value = p_value;
		emit_changed();
	}
	Variant get_saved_value() const { return saved_value; }

	void set_variable(const String &p_value) {
		variable = p_value;
		emit_changed();
	}
	String get_variable() const { return variable; }

	virtual Variant get_value(Object *p_agent, const Ref<Blackboard> &p_blackboard, const Variant &p_default = Variant());

	BBParam();
};

#endif // BB_PARAM_H