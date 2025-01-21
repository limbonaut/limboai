/**
 * bb_basis.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_BASIS_H
#define BB_BASIS_H

#include "bb_param.h"

class BBBasis : public BBParam {
	GDCLASS(BBBasis, BBParam);

protected:
	static void _bind_methods() {}

	virtual Variant::Type get_type() const override { return Variant::BASIS; }
};

#endif // BB_BASIS_H
