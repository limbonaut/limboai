/**
 * test_bt_player.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#pragma once

#include "lambda_callable.h"
#include "limbo_test.h"

#include "modules/limboai/bt/behavior_tree.h"
#include "modules/limboai/bt/bt_player.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"

namespace TestBTPlayer {

TEST_CASE("[Modules][LimboAI] BTPlayer basic functionality") {
	BTPlayer *bt_player = memnew(BTPlayer);

	SUBCASE("Initial state") {
		CHECK(bt_player->get_behavior_tree().is_null());
		CHECK(bt_player->get_bt_instance().is_null());
		CHECK(bt_player->get_blackboard().is_valid());
		CHECK(bt_player->get_active() == true);
		CHECK(bt_player->get_update_mode() == BTPlayer::PHYSICS);
	}

	SUBCASE("Property setters and getters") {
		// Test update mode
		bt_player->set_update_mode(BTPlayer::IDLE);
		CHECK(bt_player->get_update_mode() == BTPlayer::IDLE);

		bt_player->set_update_mode(BTPlayer::MANUAL);
		CHECK(bt_player->get_update_mode() == BTPlayer::MANUAL);

		// Test active state
		bt_player->set_active(false);
		CHECK(bt_player->get_active() == false);

		bt_player->set_active(true);
		CHECK(bt_player->get_active() == true);

		// Test agent node path
		NodePath test_path("TestAgent");
		bt_player->set_agent_node(test_path);
		CHECK(bt_player->get_agent_node() == test_path);
	}

	SUBCASE("Behavior tree setting without scene") {
		Ref<BehaviorTree> bt = memnew(BehaviorTree);
		bt_player->set_behavior_tree(bt);
		CHECK(bt_player->get_behavior_tree() == bt);
		CHECK(bt_player->get_bt_instance().is_null()); // not intantiated until scene root is ready
	}

	SUBCASE("Null behavior tree handling") {
		bt_player->set_behavior_tree(Ref<BehaviorTree>());
		CHECK(bt_player->get_behavior_tree().is_null());
		CHECK(bt_player->get_bt_instance().is_null());
	}

	SUBCASE("Error handling with null bt_instance") {
		CHECK(bt_player->get_bt_instance().is_null());
		ERR_PRINT_OFF;
		bt_player->update(0.01666); // Should not crash
		bt_player->restart(); // Should fail gracefully
		ERR_PRINT_ON;
	}

	memdelete(bt_player);
}

TEST_CASE("[SceneTree][LimboAI] BTPlayer lifecycle scenarios") {
	// Setup.
	Node *scene_root = memnew(Node);

	Ref<BehaviorTree> bt = memnew(BehaviorTree);

	Ref<BTAction> test_action = memnew(BTAction);
	bt->set_root_task(test_action);

	BTPlayer *bt_player = memnew(BTPlayer);

	SUBCASE("Scenario 1: Typical scene load") {
		CHECK(bt_player->get_bt_instance().is_null());

		bt_player->set_behavior_tree(bt);
		CHECK(bt_player->get_bt_instance().is_null());

		scene_root->add_child(bt_player);
		bt_player->set_owner(scene_root);
		CHECK(bt_player->get_bt_instance().is_null());

		// BTPlayer should NOT initialize in NOTIFICATION_READY, but wait for scene root.
		bt_player->connect("ready",
				memnew(LambdaCallable([bt_player]() {
					CHECK(bt_player->get_bt_instance().is_null());
				})));

		// NOTE: Automatic initialization should happen after scene root propagates NOTIFICATION_READY.
		SceneTree::get_singleton()->get_root()->add_child(scene_root);
		CHECK(bt_player->get_bt_instance().is_valid());
	}

	SUBCASE("Scenario 2: With scene root hint") {
		CHECK(bt_player->get_bt_instance().is_null());

		bt_player->set_behavior_tree(bt);
		CHECK(bt_player->get_bt_instance().is_null());

		// Setting scene root hint works as alternative to setting owner.
		Node *alt_root = memnew(Node);
		bt_player->set_scene_root_hint(alt_root);
		alt_root->add_child(bt_player);
		CHECK(bt_player->get_bt_instance().is_null());

		// BTPlayer should NOT initialize in NOTIFICATION_READY, but wait for scene root.
		bt_player->connect("ready",
				memnew(LambdaCallable([bt_player]() {
					CHECK(bt_player->get_bt_instance().is_null());
				})));

		scene_root->add_child(alt_root);
		CHECK(bt_player->get_bt_instance().is_null());

		// NOTE: Automatic initialization should happen after alt_scene_root propagates NOTIFICATION_READY.
		SceneTree::get_singleton()->get_root()->add_child(scene_root);
		CHECK(bt_player->get_bt_instance().is_valid());

		CHECK(bt_player->get_bt_instance()->get_root_task()->get_scene_root() == alt_root);
	}

	SUBCASE("Scenario 3: Setting behavior tree after NOTIFICATION_READY.") {
		CHECK(bt_player->get_bt_instance().is_null());

		scene_root->add_child(bt_player);
		bt_player->set_owner(scene_root);
		CHECK(bt_player->get_bt_instance().is_null());

		// BTPlayer should NOT initialize in NOTIFICATION_READY, but wait for scene root and behavior tree to be set.
		bt_player->connect("ready",
				memnew(LambdaCallable([bt_player]() {
					CHECK(bt_player->get_bt_instance().is_null());
				})));

		SceneTree::get_singleton()->get_root()->add_child(scene_root);
		CHECK(bt_player->get_bt_instance().is_null()); // behavior tree is not set yet

		bt_player->set_behavior_tree(bt); // should initialize here
		CHECK(bt_player->get_bt_instance().is_valid());
	}

	SUBCASE("Scenario 4: Adding BTPlayer after scene root (unlikely edge case)") {
		SceneTree::get_singleton()->get_root()->add_child(scene_root);
		CHECK(bt_player->get_bt_instance().is_null());

		bt_player->set_behavior_tree(bt);
		CHECK(bt_player->get_bt_instance().is_null());

		bt_player->set_scene_root_hint(scene_root);
		CHECK(bt_player->get_bt_instance().is_null());

		scene_root->add_child(bt_player);
		CHECK(bt_player->get_bt_instance().is_valid());
	}

	SUBCASE("Scenario 5: Activate BTPlayer after NOTIFICATION_READY") {
		CHECK(bt_player->get_bt_instance().is_null());

		bt_player->set_behavior_tree(bt);
		CHECK(bt_player->get_bt_instance().is_null());

		bt_player->set_active(false);

		scene_root->add_child(bt_player);
		bt_player->set_owner(scene_root);
		CHECK(bt_player->get_bt_instance().is_null());

		// BTPlayer should NOT initialize in NOTIFICATION_READY
		bt_player->connect("ready",
				memnew(LambdaCallable([bt_player]() {
					CHECK(bt_player->get_bt_instance().is_null());
				})));

		// NOTE: Initialization should happen after first activation.
		SceneTree::get_singleton()->get_root()->add_child(scene_root);
		CHECK(bt_player->get_bt_instance().is_null());

		bt_player->set_active(true);
		CHECK(bt_player->get_bt_instance().is_valid());
	}

	// Clean up
	scene_root->queue_free();
}

TEST_CASE("[SceneTree][LimboAI] BTPlayer blackboard persistence") {
	// Setup.
	Node *scene_root = memnew(Node);

	Ref<BehaviorTree> bt = memnew(BehaviorTree);
	Ref<BTAction> test_action = memnew(BTAction);
	bt->set_root_task(test_action);

	BTPlayer *bt_player = memnew(BTPlayer);

	SUBCASE("Variables set before initialization should persist through initialization") {
		bt_player->get_blackboard()->set_var("test_variable", 123);

		bt_player->set_behavior_tree(bt);
		scene_root->add_child(bt_player);
		bt_player->set_owner(scene_root);
		SceneTree::get_singleton()->get_root()->add_child(scene_root);

		CHECK(bt_player->get_bt_instance()->get_blackboard() == bt_player->get_blackboard());
		CHECK(bt_player->get_bt_instance()->get_blackboard()->get_var("test_variable") == Variant(123));
	}

	// Clean up
	scene_root->queue_free();
}

} // namespace TestBTPlayer
