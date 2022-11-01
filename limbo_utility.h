/* limbo_utility.h */

#ifndef LIMBO_UTILITY_H
#define LIMBO_UTILITY_H

#include "core/object.h"

class LimboUtility : public Object {
	GDCLASS(LimboUtility, Object);

protected:
	static LimboUtility *singleton;
	static void _bind_methods();

public:
	static LimboUtility *get_singleton();

	String decorate_var(String p_variable) const;
	String get_status_name(int p_status) const;

	LimboUtility();
	~LimboUtility();
};

#endif // LIMBO_UTILITY_H