/* bb_transform.h */

#ifndef BB_TRANSFORM_H
#define BB_TRANSFORM_H

#include "bb_param.h"
#include "core/object/object.h"

class BBTransform : public BBParam {
	GDCLASS(BBTransform, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::TRANSFORM3D; }
};

#endif // BB_TRANSFORM_H