/* bb_string.h */

#ifndef BB_STRING_NAME_H
#define BB_STRING_NAME_H

#include "bb_param.h"
#include "core/object/object.h"

class BBStringName : public BBParam {
	GDCLASS(BBStringName, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::STRING_NAME; }
};

#endif // BB_STRING_H