/* bb_vector3.h */

#ifndef BB_VECTOR3_H
#define BB_VECTOR3_H

#include "bb_param.h"
#include "core/object.h"

class BBVector3 : public BBParam {
	GDCLASS(BBVector3, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::VECTOR3; }
};

#endif // BB_VECTOR3_H