/* bb_int.h */

#ifndef BB_INT_H
#define BB_INT_H

#include "bb_param.h"
#include "core/object.h"

class BBInt : public BBParam {
	GDCLASS(BBInt, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::INT; }
};

#endif // BB_INT_H