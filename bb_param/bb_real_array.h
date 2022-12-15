/* bb_real_array.h */

#ifndef BB_REAL_ARRAY_H
#define BB_REAL_ARRAY_H

#include "bb_param.h"
#include "core/object/object.h"

class BBRealArray : public BBParam {
	GDCLASS(BBRealArray, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::PACKED_FLOAT64_ARRAY; }
};

#endif // BB_REAL_ARRAY_H