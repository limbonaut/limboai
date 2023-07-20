/* bb_array.h */

#ifndef BB_ARRAY_H
#define BB_ARRAY_H

#include "bb_param.h"
#include "core/object/object.h"

class BBArray : public BBParam {
	GDCLASS(BBArray, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::ARRAY; }
};

#endif // BB_ARRAY_H