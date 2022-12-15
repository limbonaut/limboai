/* bb_basis.h */

#ifndef BB_BASIS_H
#define BB_BASIS_H

#include "bb_param.h"
#include "core/object/object.h"

class BBBasis : public BBParam {
	GDCLASS(BBBasis, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::BASIS; }
};

#endif // BB_BASIS_H