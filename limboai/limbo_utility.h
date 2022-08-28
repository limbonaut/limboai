/* limbo_utility.h */

#ifndef LIMBO_UTILITY_H
#define LIMBO_UTILITY_H

#include "core/object.h"
#include "core/script_language.h"

class LimboUtility : public Object {
	GDCLASS(LimboUtility, Object);

protected:
	static void _bind_methods();

public:
	static String get_script_class(const Ref<Script> &p_script);
};

#endif // LIMBO_UTILITY_H