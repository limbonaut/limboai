/**
 * behavior_tree.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BEHAVIOR_TREE_H
#define BEHAVIOR_TREE_H

#include "core/io/resource.h"

#include "modules/limboai/blackboard/blackboard.h"
#include "tasks/bt_task.h"

class BehaviorTree : public Resource {
	GDCLASS(BehaviorTree, Resource);

private:
	String description;
	Ref<BTTask> root_task;

protected:
	static void _bind_methods();

public:
	virtual bool editor_can_reload_from_file() override { return false; }

	void set_description(String p_value) {
		description = p_value;
		emit_changed();
	}
	String get_description() const { return description; }

	void set_root_task(const Ref<BTTask> &p_value) {
		root_task = p_value;
		emit_changed();
	}
	Ref<BTTask> get_root_task() const { return root_task; }

	// void init();
	Ref<BehaviorTree> clone() const;
	Ref<BTTask> instantiate(Node *p_agent, const Ref<Blackboard> &p_blackboard) const;
};

#endif // BEHAVIOR_TREE_H