/**
 * test_hsm.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_HSM_H
#define TEST_HSM_H

#include "limbo_test.h"

#include "modules/limboai/hsm/limbo_hsm.h"
#include "modules/limboai/hsm/limbo_state.h"

#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/os/memory.h"
#include "core/variant/variant.h"
#include "scene/main/scene_tree.h"

namespace TestHSM {

inline void wire_callbacks(LimboState *p_state, Ref<CallbackCounter> p_entries_counter, Ref<CallbackCounter> p_updates_counter, Ref<CallbackCounter> p_exits_counter) {
	p_state->call_on_enter(callable_mp(p_entries_counter.ptr(), &CallbackCounter::callback));
	p_state->call_on_update(callable_mp(p_updates_counter.ptr(), &CallbackCounter::callback_delta));
	p_state->call_on_exit(callable_mp(p_exits_counter.ptr(), &CallbackCounter::callback));
}

void _on_enter_dispatch(LimboState *p_state, StringName p_event) {
	p_state->dispatch(p_event);
}

void _on_enter_set_initial_state(LimboHSM *p_state, LimboState *p_initial) {
	p_state->set_initial_state(p_initial);
}

void _on_enter_get_cargo(LimboState *p_state, const Variant &expected_cargo) {
	CHECK(p_state->get_cargo() == expected_cargo);
}

// Helper function to simulate scene tree idle process notifications
// Recursively sends NOTIFICATION_PROCESS to nodes that have process mode enabled
void _simulate_scene_tree_idle_process(Node *p_node) {
	if (p_node->is_processing()) {
		p_node->notification(Node::NOTIFICATION_PROCESS);
	}
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = p_node->get_child(i);
		_simulate_scene_tree_idle_process(child);
	}
}

// Helper function to simulate scene tree physics process notifications
// Recursively sends NOTIFICATION_PHYSICS_PROCESS to nodes that have physics process mode enabled
void _simulate_scene_tree_physics_process(Node *p_node) {
	if (p_node->is_physics_processing()) {
		p_node->notification(Node::NOTIFICATION_PHYSICS_PROCESS);
	}
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = p_node->get_child(i);
		_simulate_scene_tree_physics_process(child);
	}
}

class TestGuard : public RefCounted {
	GDCLASS(TestGuard, RefCounted);

public:
	bool permitted_to_enter = false;
	bool can_enter() { return permitted_to_enter; }
};

TEST_CASE("[Modules][LimboAI] HSM") {
	Ref<CallbackCounter> hsm_entries = memnew(CallbackCounter);
	Ref<CallbackCounter> hsm_exits = memnew(CallbackCounter);
	Ref<CallbackCounter> hsm_updates = memnew(CallbackCounter);
	Ref<CallbackCounter> alpha_entries = memnew(CallbackCounter);
	Ref<CallbackCounter> alpha_exits = memnew(CallbackCounter);
	Ref<CallbackCounter> alpha_updates = memnew(CallbackCounter);
	Ref<CallbackCounter> beta_entries = memnew(CallbackCounter);
	Ref<CallbackCounter> beta_exits = memnew(CallbackCounter);
	Ref<CallbackCounter> beta_updates = memnew(CallbackCounter);
	Ref<CallbackCounter> nested_entries = memnew(CallbackCounter);
	Ref<CallbackCounter> nested_exits = memnew(CallbackCounter);
	Ref<CallbackCounter> nested_updates = memnew(CallbackCounter);
	Ref<CallbackCounter> gamma_entries = memnew(CallbackCounter);
	Ref<CallbackCounter> gamma_exits = memnew(CallbackCounter);
	Ref<CallbackCounter> gamma_updates = memnew(CallbackCounter);
	Ref<CallbackCounter> delta_entries = memnew(CallbackCounter);
	Ref<CallbackCounter> delta_exits = memnew(CallbackCounter);
	Ref<CallbackCounter> delta_updates = memnew(CallbackCounter);

	Node *agent = memnew(Node);
	LimboHSM *hsm = memnew(LimboHSM);
	wire_callbacks(hsm, hsm_entries, hsm_updates, hsm_exits);
	agent->add_child(hsm);

	LimboState *state_alpha = memnew(LimboState);
	wire_callbacks(state_alpha, alpha_entries, alpha_updates, alpha_exits);
	LimboState *state_beta = memnew(LimboState);
	wire_callbacks(state_beta, beta_entries, beta_updates, beta_exits);
	LimboHSM *nested_hsm = memnew(LimboHSM);
	wire_callbacks(nested_hsm, nested_entries, nested_updates, nested_exits);
	LimboState *state_gamma = memnew(LimboState);
	wire_callbacks(state_gamma, gamma_entries, gamma_updates, gamma_exits);
	LimboState *state_delta = memnew(LimboState);
	wire_callbacks(state_delta, delta_entries, delta_updates, delta_exits);

	hsm->add_child(state_alpha);
	hsm->add_child(state_beta);
	hsm->add_child(nested_hsm);
	nested_hsm->add_child(state_gamma);
	nested_hsm->add_child(state_delta);

	hsm->add_transition(state_alpha, state_beta, "event_one");
	hsm->add_transition(state_beta, state_alpha, "event_two");
	hsm->add_transition(hsm->anystate(), nested_hsm, "goto_nested");
	nested_hsm->add_transition(state_gamma, state_delta, "goto_delta");
	nested_hsm->add_transition(state_delta, state_gamma, "goto_gamma");

	hsm->set_initial_state(state_alpha);
	Ref<Blackboard> parent_scope = memnew(Blackboard);
	hsm->initialize(agent, parent_scope);
	hsm->set_active(true);

	SUBCASE("Test has_transition() and remove_transition()") {
		CHECK(hsm->has_transition(state_alpha, "event_one"));
		CHECK(hsm->has_transition(state_beta, "event_two"));
		CHECK(hsm->has_transition(hsm->anystate(), "goto_nested"));
		CHECK_FALSE(hsm->has_transition(state_alpha, "event_two"));
		CHECK_FALSE(hsm->has_transition(state_beta, "event_one"));
		CHECK_FALSE(hsm->has_transition(hsm->anystate(), "event_one"));

		hsm->remove_transition(state_alpha, "event_one");
		CHECK_FALSE(hsm->has_transition(state_alpha, "event_one"));
		hsm->remove_transition(state_beta, "event_two");
		CHECK_FALSE(hsm->has_transition(state_beta, "event_two"));
		hsm->remove_transition(hsm->anystate(), "goto_nested");
		CHECK_FALSE(hsm->has_transition(hsm->anystate(), "goto_nested"));
	}
	SUBCASE("Test get_root()") {
		CHECK(state_alpha->get_root() == hsm);
		CHECK(state_beta->get_root() == hsm);
		CHECK(hsm->get_root() == hsm);
	}
	SUBCASE("Test with basic workflow and transitions") {
		REQUIRE(hsm->is_active());
		REQUIRE(hsm->get_active_state() == state_alpha);
		CHECK(alpha_entries->num_callbacks == 1); // * entered
		CHECK(alpha_updates->num_callbacks == 0);
		CHECK(alpha_exits->num_callbacks == 0);
		CHECK(beta_entries->num_callbacks == 0);
		CHECK(beta_updates->num_callbacks == 0);
		CHECK(beta_exits->num_callbacks == 0);

		hsm->update(0.01666);
		CHECK(alpha_entries->num_callbacks == 1);
		CHECK(alpha_updates->num_callbacks == 1); // * updated
		CHECK(alpha_exits->num_callbacks == 0);
		CHECK(beta_entries->num_callbacks == 0);
		CHECK(beta_updates->num_callbacks == 0);
		CHECK(beta_exits->num_callbacks == 0);

		hsm->update(0.01666);
		CHECK(alpha_entries->num_callbacks == 1);
		CHECK(alpha_updates->num_callbacks == 2); // * updated x2
		CHECK(alpha_exits->num_callbacks == 0);
		CHECK(beta_entries->num_callbacks == 0);
		CHECK(beta_updates->num_callbacks == 0);
		CHECK(beta_exits->num_callbacks == 0);

		hsm->dispatch("event_one");
		REQUIRE(hsm->get_active_state() == state_beta);
		CHECK(alpha_entries->num_callbacks == 1);
		CHECK(alpha_updates->num_callbacks == 2);
		CHECK(alpha_exits->num_callbacks == 1); // * (1) exited
		CHECK(beta_entries->num_callbacks == 1); // * (2) entered
		CHECK(beta_updates->num_callbacks == 0);
		CHECK(beta_exits->num_callbacks == 0);

		hsm->update(0.01666);
		CHECK(alpha_entries->num_callbacks == 1);
		CHECK(alpha_updates->num_callbacks == 2);
		CHECK(alpha_exits->num_callbacks == 1);
		CHECK(beta_entries->num_callbacks == 1);
		CHECK(beta_updates->num_callbacks == 1); // * updated
		CHECK(beta_exits->num_callbacks == 0);

		hsm->update(0.01666);
		CHECK(alpha_entries->num_callbacks == 1);
		CHECK(alpha_updates->num_callbacks == 2);
		CHECK(alpha_exits->num_callbacks == 1);
		CHECK(beta_entries->num_callbacks == 1);
		CHECK(beta_updates->num_callbacks == 2); // * updated x2
		CHECK(beta_exits->num_callbacks == 0);

		hsm->dispatch("event_two");
		REQUIRE(hsm->get_active_state() == state_alpha);
		CHECK(alpha_entries->num_callbacks == 2); // * (2) entered
		CHECK(alpha_updates->num_callbacks == 2);
		CHECK(alpha_exits->num_callbacks == 1);
		CHECK(beta_entries->num_callbacks == 1);
		CHECK(beta_updates->num_callbacks == 2);
		CHECK(beta_exits->num_callbacks == 1); // * (1) exited

		hsm->update(0.01666);
		CHECK(alpha_entries->num_callbacks == 2);
		CHECK(alpha_updates->num_callbacks == 3); // * updated
		CHECK(alpha_exits->num_callbacks == 1);
		CHECK(beta_entries->num_callbacks == 1);
		CHECK(beta_updates->num_callbacks == 2);
		CHECK(beta_exits->num_callbacks == 1);

		hsm->dispatch(hsm->event_finished());
		CHECK(alpha_entries->num_callbacks == 2);
		CHECK(alpha_updates->num_callbacks == 3);
		CHECK(alpha_exits->num_callbacks == 2); // * exited
		CHECK(beta_entries->num_callbacks == 1);
		CHECK(beta_updates->num_callbacks == 2);
		CHECK(beta_exits->num_callbacks == 1);
		CHECK_FALSE(hsm->is_active()); // * not active
		CHECK(hsm->get_active_state() == nullptr);
	}
	SUBCASE("Test dispatch() inside _enter()") {
		state_beta->connect("entered",
				callable_mp_static(_on_enter_dispatch).bind(state_beta, "event_two"));
		hsm->dispatch("event_one");
		REQUIRE(hsm->get_active_state() == state_alpha);
		CHECK(alpha_entries->num_callbacks == 2);
		CHECK(alpha_updates->num_callbacks == 0);
		CHECK(alpha_exits->num_callbacks == 1);
		CHECK(beta_entries->num_callbacks == 1);
		CHECK(beta_updates->num_callbacks == 0);
		CHECK(beta_exits->num_callbacks == 1);
	}
	SUBCASE("Test get_cargo() inside and outside _enter()") {
		const Variant cargo = "some data";
		state_beta->connect("entered",
				callable_mp_static(_on_enter_get_cargo).bind(state_beta, cargo));
		hsm->dispatch("event_one", cargo);
		REQUIRE(hsm->get_active_state() == state_beta);
		CHECK(state_beta->get_cargo() == Variant()); // * pointer was dereferenced, null object is returned
		CHECK(&cargo != nullptr); // * original Variant wasn't modified and will be cleared automaticaly
	}
	SUBCASE("Test setting initial_state on enter") {
		// Setting initial state on HSM enter should be allowed.
		nested_hsm->connect("entered",
				callable_mp_static(_on_enter_set_initial_state).bind(nested_hsm, state_delta));
		hsm->dispatch("goto_nested");
		REQUIRE(hsm->get_active_state() == nested_hsm);
		REQUIRE(nested_hsm->get_active_state() == state_delta);
		CHECK(delta_entries->num_callbacks == 1);
		CHECK(delta_updates->num_callbacks == 0);
		CHECK(delta_exits->num_callbacks == 0);
	}
	SUBCASE("Test change_active_state()") {
		REQUIRE(hsm->is_active());
		REQUIRE(hsm->get_active_state() == state_alpha);

		CHECK(alpha_entries->num_callbacks == 1); // * entered
		CHECK(alpha_updates->num_callbacks == 0);
		CHECK(alpha_exits->num_callbacks == 0);
		CHECK(beta_entries->num_callbacks == 0);
		CHECK(beta_updates->num_callbacks == 0);
		CHECK(beta_exits->num_callbacks == 0);

		hsm->change_active_state(state_beta);
		CHECK(hsm->get_active_state() == state_beta);

		CHECK(alpha_entries->num_callbacks == 1);
		CHECK(alpha_updates->num_callbacks == 0);
		CHECK(alpha_exits->num_callbacks == 1); // * exited
		CHECK(beta_entries->num_callbacks == 1); // * entered
		CHECK(beta_updates->num_callbacks == 0);
		CHECK(beta_exits->num_callbacks == 0);

		hsm->change_active_state(state_beta); // * should exit and re-enter
		CHECK(hsm->get_active_state() == state_beta);

		CHECK(alpha_entries->num_callbacks == 1);
		CHECK(alpha_updates->num_callbacks == 0);
		CHECK(alpha_exits->num_callbacks == 1);
		CHECK(beta_entries->num_callbacks == 2); // * re-entered
		CHECK(beta_updates->num_callbacks == 0);
		CHECK(beta_exits->num_callbacks == 1); // * exited
	}
	SUBCASE("Test transition with state-wide guard") {
		Ref<TestGuard> guard = memnew(TestGuard);
		state_beta->set_guard(callable_mp(guard.ptr(), &TestGuard::can_enter));

		SUBCASE("When entry is permitted") {
			guard->permitted_to_enter = true;
			hsm->dispatch("event_one");
			CHECK(hsm->get_active_state() == state_beta);
			CHECK(alpha_exits->num_callbacks == 1);
			CHECK(beta_entries->num_callbacks == 1);
		}
		SUBCASE("When entry is not permitted") {
			guard->permitted_to_enter = false;
			hsm->dispatch("event_one");
			CHECK(hsm->get_active_state() == state_alpha);
			CHECK(alpha_exits->num_callbacks == 0);
			CHECK(beta_entries->num_callbacks == 0);
		}
	}
	SUBCASE("Test transition with transition-scoped guard") {
		Ref<TestGuard> guard = memnew(TestGuard);
		hsm->add_transition(state_alpha, state_beta, "guarded_transition", callable_mp(guard.ptr(), &TestGuard::can_enter));

		SUBCASE("When entry is permitted") {
			guard->permitted_to_enter = true;
			hsm->dispatch("guarded_transition");
			CHECK(hsm->get_active_state() == state_beta);
			CHECK(alpha_exits->num_callbacks == 1);
			CHECK(beta_entries->num_callbacks == 1);
		}
		SUBCASE("When entry is not permitted") {
			guard->permitted_to_enter = false;
			hsm->dispatch("guarded_transition");
			CHECK(hsm->get_active_state() == state_alpha);
			CHECK(alpha_exits->num_callbacks == 0);
			CHECK(beta_entries->num_callbacks == 0);
		}
	}
	SUBCASE("When there is no transition for given event") {
		hsm->dispatch("not_found");
		CHECK(alpha_exits->num_callbacks == 0);
		CHECK(beta_entries->num_callbacks == 0);
		CHECK(hsm->is_active());
		CHECK(hsm->get_active_state() == state_alpha);
	}
	SUBCASE("Check if parent scope is accessible") {
		parent_scope->set_var("parent_var", 100);
		CHECK(state_alpha->get_blackboard()->get_parent() == parent_scope);
		CHECK(state_beta->get_blackboard()->get_parent() == parent_scope);
		CHECK(state_alpha->get_blackboard()->get_var("parent_var", Variant()) == Variant(100));
	}
	SUBCASE("Test flow with a nested HSM, and test dispatch() from nested states") {
		state_gamma->dispatch("goto_nested");
		CHECK(hsm->get_leaf_state() == state_gamma);
		CHECK(nested_entries->num_callbacks == 1);
		CHECK(nested_updates->num_callbacks == 0);
		CHECK(nested_exits->num_callbacks == 0);
		CHECK(gamma_entries->num_callbacks == 1);
		CHECK(gamma_updates->num_callbacks == 0);
		CHECK(gamma_exits->num_callbacks == 0);

		hsm->update(0.01666);
		CHECK(nested_entries->num_callbacks == 1);
		CHECK(nested_updates->num_callbacks == 1);
		CHECK(nested_exits->num_callbacks == 0);
		CHECK(gamma_entries->num_callbacks == 1);
		CHECK(gamma_updates->num_callbacks == 1);
		CHECK(gamma_exits->num_callbacks == 0);

		state_gamma->dispatch("goto_delta");
		CHECK(hsm->get_leaf_state() == state_delta);
		CHECK(nested_entries->num_callbacks == 1);
		CHECK(nested_updates->num_callbacks == 1);
		CHECK(nested_exits->num_callbacks == 0);
		CHECK(gamma_entries->num_callbacks == 1);
		CHECK(gamma_updates->num_callbacks == 1);
		CHECK(gamma_exits->num_callbacks == 1);
		CHECK(delta_entries->num_callbacks == 1);
		CHECK(delta_updates->num_callbacks == 0);
		CHECK(delta_exits->num_callbacks == 0);

		state_delta->dispatch(hsm->event_finished());
		CHECK(nested_entries->num_callbacks == 1);
		CHECK(nested_updates->num_callbacks == 1);
		CHECK(nested_exits->num_callbacks == 1);
		CHECK(gamma_entries->num_callbacks == 1);
		CHECK(gamma_updates->num_callbacks == 1);
		CHECK(gamma_exits->num_callbacks == 1);
		CHECK(delta_entries->num_callbacks == 1);
		CHECK(delta_updates->num_callbacks == 0);
		CHECK(delta_exits->num_callbacks == 1);
		CHECK(hsm->is_active() == false);
		CHECK(hsm->get_leaf_state() == hsm);
	}
	SUBCASE("Test get_root()") {
		CHECK(hsm->get_root() == hsm);
		CHECK(state_alpha->get_root() == hsm);
		CHECK(state_beta->get_root() == hsm);
		CHECK(nested_hsm->get_root() == hsm);
		CHECK(state_delta->get_root() == hsm);
		CHECK(state_gamma->get_root() == hsm);
	}
	SUBCASE("Test restart()") {
		REQUIRE(hsm->is_active());
		REQUIRE(hsm->get_active_state() == state_alpha);
		hsm->restart();
		CHECK(alpha_exits->num_callbacks == 1); // * exited
		CHECK(alpha_entries->num_callbacks == 2); // * re-entered
	}
	SUBCASE("Test EVENT_FINISHED should be unique") {
		CHECK(state_alpha->event_finished() != state_beta->event_finished());
		CHECK(state_alpha->event_finished() != state_gamma->event_finished());
		CHECK(state_alpha->event_finished() != state_delta->event_finished());
		CHECK(state_beta->event_finished() != state_gamma->event_finished());
		CHECK(state_beta->event_finished() != state_delta->event_finished());
		CHECK(state_gamma->event_finished() != state_delta->event_finished());
		CHECK(hsm->event_finished() != state_alpha->event_finished());
		CHECK(hsm->event_finished() != state_beta->event_finished());
		CHECK(hsm->event_finished() != state_gamma->event_finished());
		CHECK(hsm->event_finished() != state_delta->event_finished());
	}
	SUBCASE("Test multi-level nested HSM state update is called only once on automatic update") {
		// This test verifies that in a multi-level nested HSM structure, each state's update
		// method is called exactly once per process frame, preventing duplicate updates.

		hsm->set_update_mode(LimboHSM::UpdateMode::PHYSICS);

		hsm->dispatch("goto_nested");
		REQUIRE(hsm->get_active_state() == nested_hsm);
		REQUIRE(nested_hsm->get_active_state() == state_gamma); // Default initial state in nested HSM

		// Verify initial state: no updates have been called yet
		int baseline_hsm_updates = hsm_updates->num_callbacks;
		int baseline_nested_updates = nested_updates->num_callbacks;
		int baseline_gamma_updates = gamma_updates->num_callbacks;

		CHECK(baseline_hsm_updates == 0);
		CHECK(baseline_nested_updates == 0);
		CHECK(baseline_gamma_updates == 0);

		// Test 1: Physics process should trigger updates in PHYSICS mode
		_simulate_scene_tree_physics_process(hsm);
		CHECK(hsm_updates->num_callbacks == baseline_hsm_updates + 1); // Root HSM updated once
		CHECK(nested_updates->num_callbacks == baseline_nested_updates + 1); // Nested HSM updated once
		CHECK(gamma_updates->num_callbacks == baseline_gamma_updates + 1); // Active leaf state updated once

		// Test 2: Idle process should NOT trigger updates in PHYSICS mode
		_simulate_scene_tree_idle_process(hsm);
		CHECK(hsm_updates->num_callbacks == baseline_hsm_updates + 1); // No additional updates
		CHECK(nested_updates->num_callbacks == baseline_nested_updates + 1); // No additional updates
		CHECK(gamma_updates->num_callbacks == baseline_gamma_updates + 1); // No additional updates

		// Test 3: Verify consistent single-update behavior on subsequent physics frames
		int before_second_frame_hsm = hsm_updates->num_callbacks;
		int before_second_frame_nested = nested_updates->num_callbacks;
		int before_second_frame_gamma = gamma_updates->num_callbacks;

		_simulate_scene_tree_physics_process(hsm);

		CHECK(hsm_updates->num_callbacks == before_second_frame_hsm + 1); // Root HSM updated once more
		CHECK(nested_updates->num_callbacks == before_second_frame_nested + 1); // Nested HSM updated once more
		CHECK(gamma_updates->num_callbacks == before_second_frame_gamma + 1); // Gamma state updated once more

		// Test 4: Verify behavior after state transition within nested HSM
		state_gamma->dispatch("goto_delta");
		REQUIRE(nested_hsm->get_active_state() == state_delta);

		int before_transition_frame_nested = nested_updates->num_callbacks;
		int before_transition_frame_gamma = gamma_updates->num_callbacks;
		int before_transition_frame_delta = delta_updates->num_callbacks;

		_simulate_scene_tree_physics_process(hsm);

		// After transition: only the new active state (delta) and its parents should update
		CHECK(nested_updates->num_callbacks == before_transition_frame_nested + 1); // Nested HSM continues to update
		CHECK(delta_updates->num_callbacks == before_transition_frame_delta + 1); // New active state updates
		CHECK(gamma_updates->num_callbacks == before_transition_frame_gamma); // Old inactive state no longer updates
	}

	memdelete(agent);
}

} //namespace TestHSM

#endif // TEST_HSM_H
