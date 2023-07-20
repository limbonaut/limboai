/* bb_color_array.h */

#ifndef BB_COLOR_ARRAY_H
#define BB_COLOR_ARRAY_H

#include "bb_param.h"
#include "core/object/object.h"

class BBColorArray : public BBParam {
	GDCLASS(BBColorArray, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::PACKED_COLOR_ARRAY; }
};

#endif // BB_COLOR_ARRAY_H