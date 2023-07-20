/* bb_rect2.h */

#ifndef BB_RECT2I_H
#define BB_RECT2I_H

#include "bb_param.h"
#include "core/object/object.h"

class BBRect2i : public BBParam {
	GDCLASS(BBRect2i, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::RECT2I; }
};

#endif // BB_RECT2I_H