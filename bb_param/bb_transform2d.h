/* bb_transform2d.h */

#ifndef BB_TRANSFORM2D_H
#define BB_TRANSFORM2D_H

#include "bb_param.h"
#include "core/object.h"

class BBTransform2D : public BBParam {
	GDCLASS(BBTransform2D, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::TRANSFORM2D; }
};

#endif // BB_TRANSFORM2D_H