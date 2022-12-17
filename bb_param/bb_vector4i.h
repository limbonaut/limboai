/* bb_vector3.h */

#ifndef BB_VECTOR4I_H
#define BB_VECTOR4I_H

#include "bb_param.h"
#include "core/object/object.h"

class BBVector4i : public BBParam {
	GDCLASS(BBVector4i, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::VECTOR4I; }
};

#endif // BB_VECTOR4I_H