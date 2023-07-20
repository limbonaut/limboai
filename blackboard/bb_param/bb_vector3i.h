/* bb_vector3.h */

#ifndef BB_VECTOR3I_H
#define BB_VECTOR3I_H

#include "bb_param.h"
#include "core/object/object.h"

class BBVector3i : public BBParam {
	GDCLASS(BBVector3i, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::VECTOR3I; }
};

#endif // BB_VECTOR3I_H