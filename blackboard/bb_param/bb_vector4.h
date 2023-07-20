/* bb_vector3.h */

#ifndef BB_VECTOR4_H
#define BB_VECTOR4_H

#include "bb_param.h"
#include "core/object/object.h"

class BBVector4 : public BBParam {
	GDCLASS(BBVector4, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::VECTOR4; }
};

#endif // BB_VECTOR4_H