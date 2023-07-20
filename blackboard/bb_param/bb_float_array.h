/* bb_real_array.h */

#ifndef BB_FLOAT_ARRAY_H
#define BB_FLOAT_ARRAY_H

#include "bb_param.h"
#include "core/object/object.h"

class BBFloatArray : public BBParam {
	GDCLASS(BBFloatArray, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::PACKED_FLOAT64_ARRAY; }
};

#endif // BB_FLOAT_ARRAY_H