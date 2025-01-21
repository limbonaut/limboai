/**
 * bb_byte_array.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_BYTE_ARRAY_H
#define BB_BYTE_ARRAY_H

#include "bb_param.h"

class BBByteArray : public BBParam {
	GDCLASS(BBByteArray, BBParam);

protected:
	static void _bind_methods() {}

	virtual Variant::Type get_type() const override { return Variant::PACKED_BYTE_ARRAY; }
};

#endif // BB_BYTE_ARRAY_H
