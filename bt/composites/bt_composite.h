/**
 * bt_composite.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_COMPOSITE_H
#define BT_COMPOSITE_H

#include "../bt_task.h"

#include "core/object/object.h"

class BTComposite : public BTTask {
	GDCLASS(BTComposite, BTTask);

public:
	virtual String get_configuration_warning() const override;
};

#endif // BT_COMPOSITE_H