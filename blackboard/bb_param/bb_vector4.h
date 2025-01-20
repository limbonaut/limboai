/**
 * bb_vector4.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_VECTOR4_H
#define BB_VECTOR4_H

#include "bb_param.h"

class BBVector4 : public BBParam {
	GDCLASS(BBVector4, BBParam);

protected:
	static void _bind_methods() {}

	virtual Variant::Type get_type() const override { return Variant::VECTOR4; }
};

#endif // BB_VECTOR4_H
