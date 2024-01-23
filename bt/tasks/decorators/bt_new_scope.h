/**
 * bt_new_scope.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_NEW_SCOPE_H
#define BT_NEW_SCOPE_H

#include "../bt_decorator.h"

#include "../../../blackboard/blackboard_source.h"

class BTNewScope : public BTDecorator {
	GDCLASS(BTNewScope, BTDecorator);
	TASK_CATEGORY(Decorators);

private:
	Ref<BlackboardSource> blackboard_source;

protected:
	static void _bind_methods();

	void set_blackboard_source(const Ref<BlackboardSource> &p_source) { blackboard_source = p_source; }
	Ref<BlackboardSource> get_blackboard_source() const { return blackboard_source; }

	virtual Status _tick(double p_delta) override;

public:
	virtual void initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard) override;
};

#endif // BT_NEW_SCOPE_H
