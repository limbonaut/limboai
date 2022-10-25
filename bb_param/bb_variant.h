/* bb_variant.h */

#ifndef BB_VARIANT_H
#define BB_VARIANT_H

#include "bb_param.h"
#include "core/object.h"
#include "core/variant.h"

class BBVariant : public BBParam {
	GDCLASS(BBVariant, BBParam);

private:
	Variant::Type type;

protected:
	static void _bind_methods();

	virtual Variant::Type get_type() const { return type; }
	void set_type(Variant::Type p_type);

public:
	BBVariant();
};

#endif // BB_VARIANT