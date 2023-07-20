/* bb_vector3_array.h */

#ifndef BB_VECTOR3_ARRAY_H
#define BB_VECTOR3_ARRAY_H

#include "bb_param.h"
#include "core/object/object.h"

class BBVector3Array : public BBParam {
	GDCLASS(BBVector3Array, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::PACKED_VECTOR3_ARRAY; }
};

#endif // BB_VECTOR3_ARRAY_H