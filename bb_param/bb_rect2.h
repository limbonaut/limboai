/* bb_rect2.h */

#ifndef BB_RECT2_H
#define BB_RECT2_H

#include "bb_param.h"
#include "core/object.h"

class BBRect2 : public BBParam {
	GDCLASS(BBRect2, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::RECT2; }
};

#endif // BB_RECT2_H