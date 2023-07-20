/* bb_int_array.h */

#ifndef BB_INT_ARRAY_H
#define BB_INT_ARRAY_H

#include "bb_param.h"
#include "core/object/object.h"

class BBIntArray : public BBParam {
	GDCLASS(BBIntArray, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::PACKED_INT64_ARRAY; }
};

#endif // BB_INT_ARRAY_H