/* bb_quat.h */

#ifndef BB_QUAT_H
#define BB_QUAT_H

#include "bb_param.h"
#include "core/object.h"

class BBQuat : public BBParam {
	GDCLASS(BBQuat, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::QUAT; }
};

#endif // BB_QUAT_H