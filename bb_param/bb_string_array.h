/* bb_string_array.h */

#ifndef BB_STRING_ARRAY_H
#define BB_STRING_ARRAY_H

#include "bb_param.h"
#include "core/object.h"

class BBStringArray : public BBParam {
	GDCLASS(BBStringArray, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::POOL_STRING_ARRAY; }
};

#endif // BB_STRING_ARRAY_H