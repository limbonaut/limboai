/**
 * bb_transform3d.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_TRANSFORM3D_H
#define BB_TRANSFORM3D_H

#include "bb_param.h"

class BBTransform3D : public BBParam {
	GDCLASS(BBTransform3D, BBParam);

protected:
	static void _bind_methods() {}

	virtual Variant::Type get_type() const override { return Variant::TRANSFORM3D; }
};

#endif // BB_TRANSFORM3D_H
