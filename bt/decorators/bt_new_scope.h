/* bt_new_scope.h */

#ifndef BT_NEW_SCOPE_H
#define BT_NEW_SCOPE_H

#include "bt_decorator.h"
#include "core/object/object.h"

#include "bt_decorator.h"
class BTNewScope : public BTDecorator {
	GDCLASS(BTNewScope, BTDecorator);

private:
	Dictionary blackboard_data;

protected:
	static void _bind_methods();

	void _set_blackboard_data(const Dictionary &p_value) { blackboard_data = p_value; }
	Dictionary _get_blackboard_data() const { return blackboard_data; }

	virtual int _tick(float p_delta) override;

public:
	virtual void initialize(Object *p_agent, const Ref<Blackboard> &p_blackboard) override;
};

#endif // BT_NEW_SCOPE_H