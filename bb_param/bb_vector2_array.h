/* bb_vector2_array.h */

#ifndef BB_VECTOR2_ARRAY_H
#define BB_VECTOR2_ARRAY_H

#include "bb_param.h"
#include "core/object.h"

class BBVector2Array : public BBParam {
	GDCLASS(BBVector2Array, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::POOL_VECTOR2_ARRAY; }
};

#endif // BB_VECTOR2_ARRAY_H