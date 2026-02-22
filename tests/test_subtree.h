/**
 * test_subtree.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_SUBTREE_H
#define TEST_SUBTREE_H

#include "limbo_test.h"

#include "modules/limboai/bt/behavior_tree.h"
#include "modules/limboai/bt/tasks/bt_task.h"
#include "modules/limboai/bt/tasks/decorators/bt_subtree.h"

namespace TestSubtree {

TEST_CASE("[Modules][LimboAI] BTSubtree") {
	ClassDB::register_class<BTTestAction>();

	Ref<BTSubtree> st = memnew(BTSubtree);
	Ref<Blackboard> bb = memnew(Blackboard);
	Node *dummy = memnew(Node);

	SUBCASE("When empty") {
		ERR_PRINT_OFF;
		st->initialize(dummy, bb, dummy);
		CHECK(st->execute(0.01666) == BTTask::FAILURE);
		ERR_PRINT_ON;
	}

	SUBCASE("With a subtree assigned") {
		Ref<BehaviorTree> bt = memnew(BehaviorTree);
		Ref<BTTestAction> task = memnew(BTTestAction(BTTask::SUCCESS));
		bt->set_root_task(task);
		st->set_subtree(bt);

		CHECK(st->get_child_count() == 0);
		st->initialize(dummy, bb, dummy);
		CHECK(st->get_child_count() == 1);
		CHECK(st->get_child(0) != task);

		Ref<BTTestAction> ta = st->get_child(0);
		REQUIRE(ta.is_valid());

		SUBCASE("When child succeeds") {
			ta->ret_status = BTTask::SUCCESS;
			CHECK(st->execute(0.01666) == BTTask::SUCCESS);
			CHECK_STATUS_ENTRIES_TICKS_EXITS(ta, BTTask::SUCCESS, 1, 1, 1);
		}
		SUBCASE("When child fails") {
			ta->ret_status = BTTask::FAILURE;
			CHECK(st->execute(0.01666) == BTTask::FAILURE);
			CHECK_STATUS_ENTRIES_TICKS_EXITS(ta, BTTask::FAILURE, 1, 1, 1);
		}
		SUBCASE("When child is running") {
			ta->ret_status = BTTask::RUNNING;
			CHECK(st->execute(0.01666) == BTTask::RUNNING);
			CHECK_STATUS_ENTRIES_TICKS_EXITS(ta, BTTask::RUNNING, 1, 1, 0);
		}
	}

	SUBCASE("Test clone() preserves derived blackboard plan") {
		Ref<BehaviorTree> bt = memnew(BehaviorTree);
		Ref<BTTestAction> task = memnew(BTTestAction(BTTask::SUCCESS));
		bt->set_root_task(task);

		// Add variables to the subtree BT's plan.
		Ref<BlackboardPlan> bt_plan = memnew(BlackboardPlan);
		BBVariable speed_var(Variant::FLOAT);
		speed_var.set_value(100.0);
		bt_plan->add_var("speed", speed_var);
		BBVariable health_var(Variant::INT);
		health_var.set_value(50);
		bt_plan->add_var("health", health_var);
		bt->set_blackboard_plan(bt_plan);

		st->set_subtree(bt);

		// Verify the subtree's plan is derived from the BT's plan.
		Ref<BlackboardPlan> st_plan = st->get("blackboard_plan");
		REQUIRE(st_plan.is_valid());
		REQUIRE(st_plan->is_derived());
		REQUIRE(st_plan->get_base_plan() == bt_plan);
		CHECK(st_plan->has_var("speed"));
		CHECK(st_plan->has_var("health"));

		// Clone the subtree.
		Ref<BTSubtree> cloned = st->clone();
		REQUIRE(cloned.is_valid());

		// Cloned subtree's plan should be derived from its BT's plan.
		Ref<BehaviorTree> cloned_bt = cloned->get_subtree();
		Ref<BlackboardPlan> cloned_plan = cloned->get("blackboard_plan");
		REQUIRE(cloned_plan.is_valid());
		CHECK(cloned_plan->is_derived());
		CHECK(cloned_plan->get_base_plan() == cloned_bt->get_blackboard_plan());

		// All variables should be present in the cloned plan.
		CHECK(cloned_plan->has_var("speed"));
		CHECK(cloned_plan->has_var("health"));
	}

	memdelete(dummy);
}

} //namespace TestSubtree

#endif // TEST_SUBTREE_H
