/* bb_int_array.h */

#ifndef BB_INT_ARRAY_H
#define BB_INT_ARRAY_H

#include "bb_param.h"
#include "core/object.h"

class BBIntArray : public BBParam {
	GDCLASS(BBIntArray, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::POOL_INT_ARRAY; }
};

#endif // BB_INT_ARRAY_H