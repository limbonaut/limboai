/**
 * test_disabled_root.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_DISABLED_ROOT_H
#define TEST_DISABLED_ROOT_H

#include "limbo_test.h"

#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/bt/behavior_tree.h"
#include "modules/limboai/bt/bt_instance.h"

namespace TestDisabledRoot {

TEST_CASE("[SceneTree][LimboAI] BTInstance when root task disabled FAILURE") {
	Ref<BehaviorTree> bt = memnew(BehaviorTree);
	Ref<BTTestAction> task = memnew(BTTestAction(BTTask::SUCCESS));
	Node *dummy = memnew(Node);

	SceneTree::get_singleton()->get_root()->add_child(dummy);
	dummy->set_owner(SceneTree::get_singleton()->get_root());
	task->set_enabled(false);
	bt->set_root_task(task);
	Ref<Blackboard> blackboard = memnew(Blackboard);
	Ref<BTInstance> bti = bt->instantiate(dummy, blackboard, dummy);

	CHECK(bti->update(0.1666) == BTTask::FAILURE);
	CHECK(task->get_status() == BTTask::FRESH);

	SceneTree::get_singleton()->get_root()->remove_child(dummy);
	memdelete(dummy);
}

} //namespace TestDisabledRoot

#endif // TEST_DISABLED_ROOT_H
