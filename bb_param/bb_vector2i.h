/* bb_vector2.h */

#ifndef BB_VECTOR2I_H
#define BB_VECTOR2I_H

#include "bb_param.h"
#include "core/object/object.h"

class BBVector2i : public BBParam {
	GDCLASS(BBVector2i, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::VECTOR2I; }
};

#endif // BB_VECTOR2I_H