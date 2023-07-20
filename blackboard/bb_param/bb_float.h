/* bb_float.h */

#ifndef BB_FLOAT_H
#define BB_FLOAT_H

#include "bb_param.h"
#include "core/object/object.h"

class BBFloat : public BBParam {
	GDCLASS(BBFloat, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::FLOAT; }
};

#endif // BB_FLOAT_H