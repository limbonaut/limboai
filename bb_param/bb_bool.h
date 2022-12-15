/* bb_bool.h */

#ifndef BB_BOOL_H
#define BB_BOOL_H

#include "bb_param.h"
#include "core/object/object.h"

class BBBool : public BBParam {
	GDCLASS(BBBool, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::BOOL; }
};

#endif // BB_BOOL_H