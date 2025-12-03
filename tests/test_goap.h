/**
 * test_goap.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_GOAP_H
#define TEST_GOAP_H

#include "limbo_test.h"

#include "modules/limboai/goap/goap_action.h"
#include "modules/limboai/goap/goap_goal.h"
#include "modules/limboai/goap/goap_planner.h"
#include "modules/limboai/goap/goap_world_state.h"

namespace TestGOAP {

// Helper function to create an action with given preconditions and effects
Ref<GOAPAction> make_action(const StringName &p_name, const Dictionary &p_preconditions, const Dictionary &p_effects, int p_cost = 1) {
	Ref<GOAPAction> action;
	action.instantiate();
	action->set_action_name(p_name);
	action->set_preconditions(p_preconditions);
	action->set_effects(p_effects);
	action->set_base_cost(p_cost);
	return action;
}

// Helper function to create a goal
Ref<GOAPGoal> make_goal(const StringName &p_name, const Dictionary &p_target_state) {
	Ref<GOAPGoal> goal;
	goal.instantiate();
	goal->set_goal_name(p_name);
	goal->set_target_state(p_target_state);
	return goal;
}

// Helper function to create a world state
Ref<GOAPWorldState> make_world_state(const Dictionary &p_state) {
	Ref<GOAPWorldState> state;
	state.instantiate();
	state->set_state(p_state);
	return state;
}

TEST_CASE("[Modules][LimboAI] GOAPWorldState") {
	Ref<GOAPWorldState> state;
	state.instantiate();

	SUBCASE("Test set/get facts") {
		state->set_fact("has_weapon", true);
		state->set_fact("ammo_count", 10);
		state->set_fact("health", 100.0);

		CHECK(state->has_fact("has_weapon"));
		CHECK_EQ(state->get_fact("has_weapon"), Variant(true));
		CHECK_EQ(state->get_fact("ammo_count"), Variant(10));
		CHECK_EQ(state->get_fact("health"), Variant(100.0));
		CHECK_FALSE(state->has_fact("nonexistent"));
		CHECK_EQ(state->get_fact("nonexistent", Variant("default")), Variant("default"));
	}

	SUBCASE("Test erase_fact") {
		state->set_fact("test", 123);
		CHECK(state->has_fact("test"));
		state->erase_fact("test");
		CHECK_FALSE(state->has_fact("test"));
	}

	SUBCASE("Test clear") {
		state->set_fact("a", 1);
		state->set_fact("b", 2);
		state->clear();
		CHECK_FALSE(state->has_fact("a"));
		CHECK_FALSE(state->has_fact("b"));
	}

	SUBCASE("Test duplicate") {
		state->set_fact("value", 42);
		Ref<GOAPWorldState> copy = state->duplicate();
		CHECK_EQ(copy->get_fact("value"), Variant(42));

		// Modify copy should not affect original
		copy->set_fact("value", 100);
		CHECK_EQ(state->get_fact("value"), Variant(42));
		CHECK_EQ(copy->get_fact("value"), Variant(100));
	}

	SUBCASE("Test satisfies - exact match") {
		state->set_fact("a", true);
		state->set_fact("b", false);

		Dictionary goal_dict;
		goal_dict["a"] = true;
		goal_dict["b"] = false;
		Ref<GOAPWorldState> goal_state = make_world_state(goal_dict);

		CHECK(state->satisfies(goal_state));
	}

	SUBCASE("Test satisfies - partial match") {
		state->set_fact("a", true);
		state->set_fact("b", false);
		state->set_fact("c", 100);

		// Goal only cares about 'a'
		Dictionary goal_dict;
		goal_dict["a"] = true;
		Ref<GOAPWorldState> goal_state = make_world_state(goal_dict);

		CHECK(state->satisfies(goal_state));
	}

	SUBCASE("Test satisfies - mismatch") {
		state->set_fact("a", true);

		Dictionary goal_dict;
		goal_dict["a"] = false;
		Ref<GOAPWorldState> goal_state = make_world_state(goal_dict);

		CHECK_FALSE(state->satisfies(goal_state));
	}

	SUBCASE("Test satisfies - missing fact") {
		state->set_fact("a", true);

		Dictionary goal_dict;
		goal_dict["b"] = true; // State doesn't have 'b'
		Ref<GOAPWorldState> goal_state = make_world_state(goal_dict);

		CHECK_FALSE(state->satisfies(goal_state));
	}

	SUBCASE("Test distance_to - all matched") {
		state->set_fact("a", true);
		state->set_fact("b", true);

		Dictionary goal_dict;
		goal_dict["a"] = true;
		goal_dict["b"] = true;
		Ref<GOAPWorldState> goal_state = make_world_state(goal_dict);

		CHECK_EQ(state->distance_to(goal_state), 0);
	}

	SUBCASE("Test distance_to - some mismatched") {
		state->set_fact("a", true);
		state->set_fact("b", false);
		state->set_fact("c", false);

		Dictionary goal_dict;
		goal_dict["a"] = true;
		goal_dict["b"] = true;
		goal_dict["c"] = true;
		Ref<GOAPWorldState> goal_state = make_world_state(goal_dict);

		CHECK_EQ(state->distance_to(goal_state), 2); // b and c are wrong
	}

	SUBCASE("Test apply_effects") {
		state->set_fact("a", false);
		state->set_fact("b", 10);

		Dictionary effects;
		effects["a"] = true;
		effects["c"] = "new_fact";

		Ref<GOAPWorldState> new_state = state->apply_effects(effects);

		// Original unchanged
		CHECK_EQ(state->get_fact("a"), Variant(false));
		CHECK_FALSE(state->has_fact("c"));

		// New state has effects applied
		CHECK_EQ(new_state->get_fact("a"), Variant(true));
		CHECK_EQ(new_state->get_fact("b"), Variant(10)); // Unchanged
		CHECK_EQ(new_state->get_fact("c"), Variant("new_fact"));
	}
}

TEST_CASE("[Modules][LimboAI] GOAPAction") {
	SUBCASE("Test is_valid - all preconditions met") {
		Dictionary preconditions;
		preconditions["has_weapon"] = true;
		preconditions["has_ammo"] = true;

		Dictionary effects;
		effects["target_dead"] = true;

		Ref<GOAPAction> shoot = make_action("Shoot", preconditions, effects);

		Dictionary state_dict;
		state_dict["has_weapon"] = true;
		state_dict["has_ammo"] = true;
		Ref<GOAPWorldState> state = make_world_state(state_dict);

		CHECK(shoot->is_valid(state));
	}

	SUBCASE("Test is_valid - precondition not met") {
		Dictionary preconditions;
		preconditions["has_weapon"] = true;

		Dictionary effects;
		effects["target_dead"] = true;

		Ref<GOAPAction> shoot = make_action("Shoot", preconditions, effects);

		Dictionary state_dict;
		state_dict["has_weapon"] = false;
		Ref<GOAPWorldState> state = make_world_state(state_dict);

		CHECK_FALSE(shoot->is_valid(state));
	}

	SUBCASE("Test is_valid - missing precondition fact") {
		Dictionary preconditions;
		preconditions["has_weapon"] = true;

		Dictionary effects;
		effects["target_dead"] = true;

		Ref<GOAPAction> shoot = make_action("Shoot", preconditions, effects);

		Dictionary state_dict;
		// State is empty - missing has_weapon
		Ref<GOAPWorldState> state = make_world_state(state_dict);

		CHECK_FALSE(shoot->is_valid(state));
	}

	SUBCASE("Test apply_effects_to_state") {
		Dictionary preconditions;
		Dictionary effects;
		effects["has_weapon"] = true;
		effects["near_weapon"] = false;

		Ref<GOAPAction> pickup = make_action("PickUpWeapon", preconditions, effects);

		Dictionary state_dict;
		state_dict["near_weapon"] = true;
		Ref<GOAPWorldState> state = make_world_state(state_dict);

		Ref<GOAPWorldState> new_state = pickup->apply_effects_to_state(state);

		CHECK_EQ(new_state->get_fact("has_weapon"), Variant(true));
		CHECK_EQ(new_state->get_fact("near_weapon"), Variant(false));
	}

	SUBCASE("Test get_cost") {
		Dictionary empty;
		Ref<GOAPAction> action = make_action("Test", empty, empty, 5);
		CHECK_EQ(action->get_cost(), 5);
	}

	SUBCASE("Test produces_fact") {
		Dictionary preconditions;
		Dictionary effects;
		effects["has_weapon"] = true;
		effects["near_weapon"] = false;

		Ref<GOAPAction> pickup = make_action("PickUpWeapon", preconditions, effects);

		CHECK(pickup->produces_fact("has_weapon"));
		CHECK(pickup->produces_fact("near_weapon"));
		CHECK_FALSE(pickup->produces_fact("has_ammo"));
	}
}

TEST_CASE("[Modules][LimboAI] GOAPGoal") {
	SUBCASE("Test is_satisfied") {
		Dictionary target_state;
		target_state["target_dead"] = true;
		Ref<GOAPGoal> goal = make_goal("KillTarget", target_state);

		Dictionary state_dict;
		state_dict["target_dead"] = true;
		state_dict["has_weapon"] = true;
		Ref<GOAPWorldState> state = make_world_state(state_dict);

		CHECK(goal->is_satisfied(state));
	}

	SUBCASE("Test is_satisfied - not met") {
		Dictionary target_state;
		target_state["target_dead"] = true;
		Ref<GOAPGoal> goal = make_goal("KillTarget", target_state);

		Dictionary state_dict;
		state_dict["target_dead"] = false;
		Ref<GOAPWorldState> state = make_world_state(state_dict);

		CHECK_FALSE(goal->is_satisfied(state));
	}

	SUBCASE("Test create_world_state") {
		Dictionary target_state;
		target_state["a"] = true;
		target_state["b"] = 42;
		Ref<GOAPGoal> goal = make_goal("Test", target_state);

		Ref<GOAPWorldState> ws = goal->create_world_state();
		CHECK_EQ(ws->get_fact("a"), Variant(true));
		CHECK_EQ(ws->get_fact("b"), Variant(42));
	}
}

TEST_CASE("[Modules][LimboAI] GOAPPlanner - Basic Planning") {
	Ref<GOAPPlanner> planner;
	planner.instantiate();

	SUBCASE("Test simple plan - single action") {
		// Goal: has_weapon = true
		// Current: has_weapon = false, near_weapon = true
		// Action: PickUpWeapon (precond: near_weapon=true, effect: has_weapon=true)

		Dictionary pickup_precond;
		pickup_precond["near_weapon"] = true;
		Dictionary pickup_effect;
		pickup_effect["has_weapon"] = true;
		Ref<GOAPAction> pickup = make_action("PickUpWeapon", pickup_precond, pickup_effect);

		TypedArray<GOAPAction> actions;
		actions.append(pickup);

		Dictionary current_dict;
		current_dict["has_weapon"] = false;
		current_dict["near_weapon"] = true;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["has_weapon"] = true;
		Ref<GOAPGoal> goal = make_goal("GetWeapon", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		CHECK_EQ(plan.size(), 1);
		Ref<GOAPAction> planned_action = plan[0];
		CHECK_EQ(planned_action->get_action_name(), StringName("PickUpWeapon"));
	}

	SUBCASE("Test multi-step plan") {
		// Goal: target_dead = true
		// Current: has_weapon = false, near_weapon = true
		// Actions:
		//   PickUpWeapon: near_weapon=true -> has_weapon=true
		//   Shoot: has_weapon=true -> target_dead=true

		Dictionary pickup_precond;
		pickup_precond["near_weapon"] = true;
		Dictionary pickup_effect;
		pickup_effect["has_weapon"] = true;
		Ref<GOAPAction> pickup = make_action("PickUpWeapon", pickup_precond, pickup_effect);

		Dictionary shoot_precond;
		shoot_precond["has_weapon"] = true;
		Dictionary shoot_effect;
		shoot_effect["target_dead"] = true;
		Ref<GOAPAction> shoot = make_action("Shoot", shoot_precond, shoot_effect);

		TypedArray<GOAPAction> actions;
		actions.append(pickup);
		actions.append(shoot);

		Dictionary current_dict;
		current_dict["has_weapon"] = false;
		current_dict["near_weapon"] = true;
		current_dict["target_dead"] = false;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["target_dead"] = true;
		Ref<GOAPGoal> goal = make_goal("KillTarget", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		CHECK_EQ(plan.size(), 2);
		Ref<GOAPAction> first = plan[0];
		Ref<GOAPAction> second = plan[1];
		CHECK_EQ(first->get_action_name(), StringName("PickUpWeapon"));
		CHECK_EQ(second->get_action_name(), StringName("Shoot"));
	}

	SUBCASE("Test goal already satisfied") {
		Dictionary shoot_precond;
		shoot_precond["has_weapon"] = true;
		Dictionary shoot_effect;
		shoot_effect["target_dead"] = true;
		Ref<GOAPAction> shoot = make_action("Shoot", shoot_precond, shoot_effect);

		TypedArray<GOAPAction> actions;
		actions.append(shoot);

		Dictionary current_dict;
		current_dict["target_dead"] = true; // Already dead!
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["target_dead"] = true;
		Ref<GOAPGoal> goal = make_goal("KillTarget", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		CHECK_EQ(plan.size(), 0); // Empty plan - goal already achieved
	}

	SUBCASE("Test impossible goal - no action produces required fact") {
		Dictionary some_precond;
		Dictionary some_effect;
		some_effect["something_else"] = true;
		Ref<GOAPAction> some_action = make_action("SomeAction", some_precond, some_effect);

		TypedArray<GOAPAction> actions;
		actions.append(some_action);

		Dictionary current_dict;
		current_dict["target_dead"] = false;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["target_dead"] = true; // No action produces this
		Ref<GOAPGoal> goal = make_goal("KillTarget", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		CHECK_EQ(plan.size(), 0); // No plan found
	}

	SUBCASE("Test empty action set") {
		TypedArray<GOAPAction> actions; // Empty

		Dictionary current_dict;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["target_dead"] = true;
		Ref<GOAPGoal> goal = make_goal("KillTarget", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		CHECK_EQ(plan.size(), 0); // No plan possible
	}
}

TEST_CASE("[Modules][LimboAI] GOAPPlanner - Cost Optimization") {
	Ref<GOAPPlanner> planner;
	planner.instantiate();

	SUBCASE("Test chooses lower cost path") {
		// Two paths to goal:
		// Path A: ExpensiveAction (cost 10)
		// Path B: CheapAction (cost 1)
		// Both produce target_dead=true

		Dictionary no_precond;

		Dictionary effect;
		effect["target_dead"] = true;

		Ref<GOAPAction> expensive = make_action("ExpensiveAction", no_precond, effect, 10);
		Ref<GOAPAction> cheap = make_action("CheapAction", no_precond, effect, 1);

		TypedArray<GOAPAction> actions;
		actions.append(expensive);
		actions.append(cheap);

		Dictionary current_dict;
		current_dict["target_dead"] = false;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["target_dead"] = true;
		Ref<GOAPGoal> goal = make_goal("KillTarget", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		CHECK_EQ(plan.size(), 1);
		Ref<GOAPAction> chosen = plan[0];
		CHECK_EQ(chosen->get_action_name(), StringName("CheapAction"));
	}

	SUBCASE("Test chooses lower cost multi-step path") {
		// Path A: Step1 (cost 5) -> Step2 (cost 5) = total 10
		// Path B: SingleStep (cost 15)

		Dictionary no_precond;
		Dictionary step1_effect;
		step1_effect["step1_done"] = true;

		Dictionary step2_precond;
		step2_precond["step1_done"] = true;
		Dictionary goal_effect;
		goal_effect["goal_achieved"] = true;

		Ref<GOAPAction> step1 = make_action("Step1", no_precond, step1_effect, 5);
		Ref<GOAPAction> step2 = make_action("Step2", step2_precond, goal_effect, 5);
		Ref<GOAPAction> single = make_action("SingleStep", no_precond, goal_effect, 15);

		TypedArray<GOAPAction> actions;
		actions.append(step1);
		actions.append(step2);
		actions.append(single);

		Dictionary current_dict;
		current_dict["step1_done"] = false;
		current_dict["goal_achieved"] = false;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["goal_achieved"] = true;
		Ref<GOAPGoal> goal = make_goal("AchieveGoal", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		CHECK_EQ(plan.size(), 2);
		Ref<GOAPAction> first = plan[0];
		Ref<GOAPAction> second = plan[1];
		CHECK_EQ(first->get_action_name(), StringName("Step1"));
		CHECK_EQ(second->get_action_name(), StringName("Step2"));
	}
}

TEST_CASE("[Modules][LimboAI] GOAPPlanner - Edge Cases") {
	Ref<GOAPPlanner> planner;
	planner.instantiate();

	SUBCASE("Test iteration limit") {
		planner->set_max_iterations(5);

		// Create a complex scenario that would take many iterations
		TypedArray<GOAPAction> actions;
		for (int i = 0; i < 20; i++) {
			Dictionary precond;
			precond[vformat("fact_%d", i)] = true;
			Dictionary effect;
			effect[vformat("fact_%d", i + 1)] = true;
			actions.append(make_action(vformat("Action%d", i), precond, effect));
		}

		Dictionary current_dict;
		current_dict["fact_0"] = true;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["fact_20"] = true;
		Ref<GOAPGoal> goal = make_goal("FarGoal", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		// Should stop early due to iteration limit
		CHECK(planner->get_last_iterations() <= 5);
	}

	SUBCASE("Test planning statistics") {
		Dictionary no_precond;
		Dictionary effect;
		effect["done"] = true;
		Ref<GOAPAction> action = make_action("DoIt", no_precond, effect);

		TypedArray<GOAPAction> actions;
		actions.append(action);

		Dictionary current_dict;
		current_dict["done"] = false;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["done"] = true;
		Ref<GOAPGoal> goal = make_goal("Finish", goal_dict);

		planner->plan(actions, current, goal);

		CHECK(planner->get_last_iterations() > 0);
		CHECK(planner->get_last_plan_time_ms() >= 0.0);
	}
}

TEST_CASE("[Modules][LimboAI] GOAPPlanner - Tactical Demo Scenario") {
	Ref<GOAPPlanner> planner;
	planner.instantiate();

	// Set up the tactical NPC scenario from the PRD
	TypedArray<GOAPAction> actions;

	// GoToWeapon
	{
		Dictionary precond;
		Dictionary effect;
		effect["near_weapon_pickup"] = true;
		actions.append(make_action("GoToWeapon", precond, effect, 3));
	}

	// PickUpWeapon
	{
		Dictionary precond;
		precond["near_weapon_pickup"] = true;
		Dictionary effect;
		effect["has_weapon"] = true;
		effect["near_weapon_pickup"] = false;
		actions.append(make_action("PickUpWeapon", precond, effect, 1));
	}

	// GoToAmmo
	{
		Dictionary precond;
		precond["has_weapon"] = true;
		Dictionary effect;
		effect["near_ammo"] = true;
		actions.append(make_action("GoToAmmo", precond, effect, 3));
	}

	// LoadWeapon
	{
		Dictionary precond;
		precond["has_weapon"] = true;
		precond["near_ammo"] = true;
		Dictionary effect;
		effect["weapon_loaded"] = true;
		actions.append(make_action("LoadWeapon", precond, effect, 1));
	}

	// ApproachTarget
	{
		Dictionary precond;
		precond["has_weapon"] = true;
		Dictionary effect;
		effect["target_in_range"] = true;
		effect["target_visible"] = true;
		actions.append(make_action("ApproachTarget", precond, effect, 4));
	}

	// Shoot
	{
		Dictionary precond;
		precond["has_weapon"] = true;
		precond["weapon_loaded"] = true;
		precond["target_in_range"] = true;
		precond["target_visible"] = true;
		Dictionary effect;
		effect["target_dead"] = true;
		actions.append(make_action("Shoot", precond, effect, 1));
	}

	SUBCASE("Test full combat plan from empty state") {
		Dictionary current_dict;
		current_dict["has_weapon"] = false;
		current_dict["weapon_loaded"] = false;
		current_dict["target_in_range"] = false;
		current_dict["target_visible"] = false;
		current_dict["target_dead"] = false;
		current_dict["near_weapon_pickup"] = false;
		current_dict["near_ammo"] = false;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["target_dead"] = true;
		Ref<GOAPGoal> goal = make_goal("KillTarget", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		// Should find a valid plan
		CHECK(plan.size() > 0);

		// Verify the last action is Shoot
		if (plan.size() > 0) {
			Ref<GOAPAction> last = plan[plan.size() - 1];
			CHECK_EQ(last->get_action_name(), StringName("Shoot"));
		}

		// Verify plan makes sense (has weapon acquisition before shooting)
		bool has_weapon_action = false;
		bool has_shoot_action = false;
		for (int i = 0; i < plan.size(); i++) {
			Ref<GOAPAction> a = plan[i];
			if (a->get_action_name() == StringName("PickUpWeapon")) {
				has_weapon_action = true;
				CHECK_FALSE(has_shoot_action); // Pickup should come before shoot
			}
			if (a->get_action_name() == StringName("Shoot")) {
				has_shoot_action = true;
			}
		}
		CHECK(has_weapon_action);
		CHECK(has_shoot_action);
	}

	SUBCASE("Test plan with weapon already equipped") {
		Dictionary current_dict;
		current_dict["has_weapon"] = true;
		current_dict["weapon_loaded"] = false;
		current_dict["target_in_range"] = false;
		current_dict["target_visible"] = false;
		current_dict["target_dead"] = false;
		current_dict["near_ammo"] = false;
		Ref<GOAPWorldState> current = make_world_state(current_dict);

		Dictionary goal_dict;
		goal_dict["target_dead"] = true;
		Ref<GOAPGoal> goal = make_goal("KillTarget", goal_dict);

		TypedArray<GOAPAction> plan = planner->plan(actions, current, goal);

		// Should not include GoToWeapon or PickUpWeapon
		for (int i = 0; i < plan.size(); i++) {
			Ref<GOAPAction> a = plan[i];
			CHECK_NE(a->get_action_name(), StringName("GoToWeapon"));
			CHECK_NE(a->get_action_name(), StringName("PickUpWeapon"));
		}
	}
}

} // namespace TestGOAP

#endif // TEST_GOAP_H
