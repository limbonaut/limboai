/* bt_subtree.h */

#ifndef BT_SUBTREE_H
#define BT_SUBTREE_H

#include "bt_new_scope.h"
#include "core/object.h"
#include "modules/limboai/bt/behavior_tree.h"

class BTSubtree : public BTNewScope {
	GDCLASS(BTSubtree, BTNewScope);

private:
	Ref<BehaviorTree> subtree;

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual int _tick(float p_delta);

public:
	void set_subtree(const Ref<BehaviorTree> &p_value) {
		subtree = p_value;
		emit_changed();
	}
	Ref<BehaviorTree> get_subtree() const { return subtree; }

	virtual Ref<BTTask> clone() const;
	virtual String get_configuration_warning() const;
};

#endif // BT_SUBTREE_H