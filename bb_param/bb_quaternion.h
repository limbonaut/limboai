/* bb_quat.h */

#ifndef BB_QUATERNION_H
#define BB_QUATERNION_H

#include "bb_param.h"
#include "core/object/object.h"

class BBQuaternion : public BBParam {
	GDCLASS(BBQuaternion, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::QUATERNION; }
};

#endif // BB_QUATERNION_H