/* bb_transform.h */

#ifndef BB_TRANSFORM_H
#define BB_TRANSFORM_H

#include "bb_param.h"
#include "core/object.h"

class BBTransform : public BBParam {
	GDCLASS(BBTransform, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::TRANSFORM; }
};

#endif // BB_TRANSFORM_H