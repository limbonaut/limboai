/* bb_transform.h */

#ifndef BB_TRANSFORM3D_H
#define BB_TRANSFORM3D_H

#include "bb_param.h"
#include "core/object/object.h"

class BBTransform3D : public BBParam {
	GDCLASS(BBTransform3D, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::TRANSFORM3D; }
};

#endif // BB_TRANSFORM3D_H