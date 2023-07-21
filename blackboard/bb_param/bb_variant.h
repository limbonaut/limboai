/**
 * bb_variant.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_VARIANT_H
#define BB_VARIANT_H

#include "bb_param.h"
#include "core/object/object.h"
#include "core/variant/variant.h"

class BBVariant : public BBParam {
	GDCLASS(BBVariant, BBParam);

private:
	Variant::Type type;

protected:
	static void _bind_methods();

	virtual Variant::Type get_type() const override { return type; }
	void set_type(Variant::Type p_type);

public:
	BBVariant();
};

#endif // BB_VARIANT