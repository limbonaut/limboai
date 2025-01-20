/**
 * bb_vector2.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_VECTOR2_H
#define BB_VECTOR2_H

#include "bb_param.h"

class BBVector2 : public BBParam {
	GDCLASS(BBVector2, BBParam);

protected:
	static void _bind_methods() {}

	virtual Variant::Type get_type() const override { return Variant::VECTOR2; }
};

#endif // BB_VECTOR2_H
