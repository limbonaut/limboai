/* bb_string.h */

#ifndef BB_STRING_H
#define BB_STRING_H

#include "bb_param.h"
#include "core/object/object.h"

class BBString : public BBParam {
	GDCLASS(BBString, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::STRING; }
};

#endif // BB_STRING_H