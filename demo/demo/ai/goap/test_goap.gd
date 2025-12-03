## GOAP Test Script
## Run this script to verify GOAP functionality in the Godot editor
## Usage: Attach to a Node and run the scene, or call run_all_tests() from editor
extends Node

var tests_passed := 0
var tests_failed := 0
var current_test := ""


func _ready() -> void:
	print("\n" + "=".repeat(60))
	print("GOAP Test Suite")
	print("=".repeat(60) + "\n")

	run_all_tests()

	print("\n" + "=".repeat(60))
	print("Results: %d passed, %d failed" % [tests_passed, tests_failed])
	print("=".repeat(60) + "\n")

	if tests_failed > 0:
		push_error("Some tests failed!")
	else:
		print("All tests passed!")


func run_all_tests() -> void:
	# WorldState tests
	test_world_state_set_get()
	test_world_state_satisfies()
	test_world_state_distance()
	test_world_state_duplicate()
	test_world_state_apply_effects()

	# Action tests
	test_action_is_valid()
	test_action_apply_effects()
	test_action_produces_fact()

	# Goal tests
	test_goal_is_satisfied()
	test_goal_create_world_state()

	# Planner tests
	test_planner_single_action()
	test_planner_multi_step()
	test_planner_goal_satisfied()
	test_planner_impossible_goal()
	test_planner_cost_optimization()
	test_planner_tactical_scenario()


# ============================================================================
# Test Helpers
# ============================================================================

func assert_true(condition: bool, message: String = "") -> void:
	if condition:
		tests_passed += 1
		print("  ✓ %s" % [current_test if message.is_empty() else message])
	else:
		tests_failed += 1
		push_error("  ✗ %s - FAILED" % [current_test if message.is_empty() else message])


func assert_eq(actual, expected, message: String = "") -> void:
	if actual == expected:
		tests_passed += 1
		print("  ✓ %s" % [current_test if message.is_empty() else message])
	else:
		tests_failed += 1
		push_error("  ✗ %s - Expected %s but got %s" % [
			current_test if message.is_empty() else message,
			str(expected),
			str(actual)
		])


func begin_test(name: String) -> void:
	current_test = name
	print("\nTest: %s" % name)


# ============================================================================
# WorldState Tests
# ============================================================================

func test_world_state_set_get() -> void:
	begin_test("WorldState set/get facts")

	var state := GOAPWorldState.new()
	state.set_fact("has_weapon", true)
	state.set_fact("ammo", 10)
	state.set_fact("health", 100.0)

	assert_true(state.has_fact("has_weapon"), "has_fact returns true for existing")
	assert_eq(state.get_fact("has_weapon"), true, "get_fact returns correct bool")
	assert_eq(state.get_fact("ammo"), 10, "get_fact returns correct int")
	assert_eq(state.get_fact("health"), 100.0, "get_fact returns correct float")
	assert_true(!state.has_fact("nonexistent"), "has_fact returns false for missing")
	assert_eq(state.get_fact("nonexistent", "default"), "default", "get_fact returns default")


func test_world_state_satisfies() -> void:
	begin_test("WorldState satisfies")

	var state := GOAPWorldState.new()
	state.set_fact("a", true)
	state.set_fact("b", false)
	state.set_fact("c", 100)

	var goal := GOAPWorldState.new()
	goal.set_fact("a", true)
	assert_true(state.satisfies(goal), "satisfies partial match")

	goal.set_fact("b", false)
	assert_true(state.satisfies(goal), "satisfies full match")

	goal.set_fact("b", true)
	assert_true(!state.satisfies(goal), "does not satisfy mismatch")


func test_world_state_distance() -> void:
	begin_test("WorldState distance_to")

	var state := GOAPWorldState.new()
	state.set_fact("a", true)
	state.set_fact("b", false)
	state.set_fact("c", false)

	var goal := GOAPWorldState.new()
	goal.set_fact("a", true)
	goal.set_fact("b", true)
	goal.set_fact("c", true)

	assert_eq(state.distance_to(goal), 2, "distance counts mismatches")

	state.set_fact("b", true)
	state.set_fact("c", true)
	assert_eq(state.distance_to(goal), 0, "distance is 0 when satisfied")


func test_world_state_duplicate() -> void:
	begin_test("WorldState duplicate")

	var state := GOAPWorldState.new()
	state.set_fact("value", 42)

	var copy := state.duplicate()
	assert_eq(copy.get_fact("value"), 42, "duplicate copies values")

	copy.set_fact("value", 100)
	assert_eq(state.get_fact("value"), 42, "original unchanged after modifying copy")
	assert_eq(copy.get_fact("value"), 100, "copy has new value")


func test_world_state_apply_effects() -> void:
	begin_test("WorldState apply_effects")

	var state := GOAPWorldState.new()
	state.set_fact("a", false)
	state.set_fact("b", 10)

	var effects := {"a": true, "c": "new"}
	var new_state := state.apply_effects(effects)

	assert_eq(state.get_fact("a"), false, "original unchanged")
	assert_eq(new_state.get_fact("a"), true, "effect applied")
	assert_eq(new_state.get_fact("b"), 10, "unaffected fact preserved")
	assert_eq(new_state.get_fact("c"), "new", "new fact added")


# ============================================================================
# Action Tests
# ============================================================================

func test_action_is_valid() -> void:
	begin_test("Action is_valid")

	var action := GOAPAction.new()
	action.preconditions = {"has_weapon": true, "has_ammo": true}

	var state := GOAPWorldState.new()
	state.set_fact("has_weapon", true)
	state.set_fact("has_ammo", true)
	assert_true(action.is_valid(state), "valid when preconditions met")

	state.set_fact("has_ammo", false)
	assert_true(!action.is_valid(state), "invalid when precondition not met")


func test_action_apply_effects() -> void:
	begin_test("Action apply_effects_to_state")

	var action := GOAPAction.new()
	action.effects = {"has_weapon": true, "near_weapon": false}

	var state := GOAPWorldState.new()
	state.set_fact("near_weapon", true)

	var new_state := action.apply_effects_to_state(state)
	assert_eq(new_state.get_fact("has_weapon"), true, "effect applied")
	assert_eq(new_state.get_fact("near_weapon"), false, "effect overwrites")


func test_action_produces_fact() -> void:
	begin_test("Action produces_fact")

	var action := GOAPAction.new()
	action.effects = {"has_weapon": true, "near_weapon": false}

	assert_true(action.produces_fact("has_weapon"), "produces_fact true for effect")
	assert_true(!action.produces_fact("has_ammo"), "produces_fact false for non-effect")


# ============================================================================
# Goal Tests
# ============================================================================

func test_goal_is_satisfied() -> void:
	begin_test("Goal is_satisfied")

	var goal := GOAPGoal.new()
	goal.target_state = {"target_dead": true}

	var state := GOAPWorldState.new()
	state.set_fact("target_dead", false)
	assert_true(!goal.is_satisfied(state), "not satisfied when false")

	state.set_fact("target_dead", true)
	assert_true(goal.is_satisfied(state), "satisfied when true")


func test_goal_create_world_state() -> void:
	begin_test("Goal create_world_state")

	var goal := GOAPGoal.new()
	goal.target_state = {"a": true, "b": 42}

	var ws := goal.create_world_state()
	assert_eq(ws.get_fact("a"), true, "world state has goal fact a")
	assert_eq(ws.get_fact("b"), 42, "world state has goal fact b")


# ============================================================================
# Planner Tests
# ============================================================================

func test_planner_single_action() -> void:
	begin_test("Planner single action plan")

	var planner := GOAPPlanner.new()

	var pickup := GOAPAction.new()
	pickup.action_name = "PickUpWeapon"
	pickup.preconditions = {"near_weapon": true}
	pickup.effects = {"has_weapon": true}

	var actions: Array[GOAPAction] = [pickup]

	var state := GOAPWorldState.new()
	state.set_fact("has_weapon", false)
	state.set_fact("near_weapon", true)

	var goal := GOAPGoal.new()
	goal.target_state = {"has_weapon": true}

	var plan := planner.plan(actions, state, goal)
	assert_eq(plan.size(), 1, "plan has 1 action")
	if plan.size() > 0:
		assert_eq(plan[0].action_name, "PickUpWeapon", "correct action in plan")


func test_planner_multi_step() -> void:
	begin_test("Planner multi-step plan")

	var planner := GOAPPlanner.new()

	var pickup := GOAPAction.new()
	pickup.action_name = "PickUpWeapon"
	pickup.preconditions = {"near_weapon": true}
	pickup.effects = {"has_weapon": true}

	var shoot := GOAPAction.new()
	shoot.action_name = "Shoot"
	shoot.preconditions = {"has_weapon": true}
	shoot.effects = {"target_dead": true}

	var actions: Array[GOAPAction] = [pickup, shoot]

	var state := GOAPWorldState.new()
	state.set_fact("has_weapon", false)
	state.set_fact("near_weapon", true)
	state.set_fact("target_dead", false)

	var goal := GOAPGoal.new()
	goal.target_state = {"target_dead": true}

	var plan := planner.plan(actions, state, goal)
	assert_eq(plan.size(), 2, "plan has 2 actions")
	if plan.size() >= 2:
		assert_eq(plan[0].action_name, "PickUpWeapon", "first action is pickup")
		assert_eq(plan[1].action_name, "Shoot", "second action is shoot")


func test_planner_goal_satisfied() -> void:
	begin_test("Planner goal already satisfied")

	var planner := GOAPPlanner.new()

	var shoot := GOAPAction.new()
	shoot.action_name = "Shoot"
	shoot.preconditions = {"has_weapon": true}
	shoot.effects = {"target_dead": true}

	var actions: Array[GOAPAction] = [shoot]

	var state := GOAPWorldState.new()
	state.set_fact("target_dead", true)  # Already dead!

	var goal := GOAPGoal.new()
	goal.target_state = {"target_dead": true}

	var plan := planner.plan(actions, state, goal)
	assert_eq(plan.size(), 0, "empty plan when goal satisfied")


func test_planner_impossible_goal() -> void:
	begin_test("Planner impossible goal")

	var planner := GOAPPlanner.new()

	var action := GOAPAction.new()
	action.action_name = "DoSomethingElse"
	action.effects = {"something_else": true}

	var actions: Array[GOAPAction] = [action]

	var state := GOAPWorldState.new()
	state.set_fact("target_dead", false)

	var goal := GOAPGoal.new()
	goal.target_state = {"target_dead": true}  # No action produces this

	var plan := planner.plan(actions, state, goal)
	assert_eq(plan.size(), 0, "empty plan when goal impossible")


func test_planner_cost_optimization() -> void:
	begin_test("Planner cost optimization")

	var planner := GOAPPlanner.new()

	var expensive := GOAPAction.new()
	expensive.action_name = "ExpensiveAction"
	expensive.effects = {"target_dead": true}
	expensive.base_cost = 10

	var cheap := GOAPAction.new()
	cheap.action_name = "CheapAction"
	cheap.effects = {"target_dead": true}
	cheap.base_cost = 1

	var actions: Array[GOAPAction] = [expensive, cheap]

	var state := GOAPWorldState.new()
	state.set_fact("target_dead", false)

	var goal := GOAPGoal.new()
	goal.target_state = {"target_dead": true}

	var plan := planner.plan(actions, state, goal)
	assert_eq(plan.size(), 1, "plan has 1 action")
	if plan.size() > 0:
		assert_eq(plan[0].action_name, "CheapAction", "chose cheaper action")


func test_planner_tactical_scenario() -> void:
	begin_test("Planner tactical scenario")

	var planner := GOAPPlanner.new()
	var actions: Array[GOAPAction] = []

	# GoToWeapon
	var go_to_weapon := GOAPAction.new()
	go_to_weapon.action_name = "GoToWeapon"
	go_to_weapon.effects = {"near_weapon_pickup": true}
	go_to_weapon.base_cost = 3
	actions.append(go_to_weapon)

	# PickUpWeapon
	var pickup := GOAPAction.new()
	pickup.action_name = "PickUpWeapon"
	pickup.preconditions = {"near_weapon_pickup": true}
	pickup.effects = {"has_weapon": true, "near_weapon_pickup": false}
	pickup.base_cost = 1
	actions.append(pickup)

	# GoToAmmo
	var go_to_ammo := GOAPAction.new()
	go_to_ammo.action_name = "GoToAmmo"
	go_to_ammo.preconditions = {"has_weapon": true}
	go_to_ammo.effects = {"near_ammo": true}
	go_to_ammo.base_cost = 3
	actions.append(go_to_ammo)

	# LoadWeapon
	var load_weapon := GOAPAction.new()
	load_weapon.action_name = "LoadWeapon"
	load_weapon.preconditions = {"has_weapon": true, "near_ammo": true}
	load_weapon.effects = {"weapon_loaded": true}
	load_weapon.base_cost = 1
	actions.append(load_weapon)

	# ApproachTarget
	var approach := GOAPAction.new()
	approach.action_name = "ApproachTarget"
	approach.preconditions = {"has_weapon": true}
	approach.effects = {"target_in_range": true, "target_visible": true}
	approach.base_cost = 4
	actions.append(approach)

	# Shoot
	var shoot := GOAPAction.new()
	shoot.action_name = "Shoot"
	shoot.preconditions = {
		"has_weapon": true,
		"weapon_loaded": true,
		"target_in_range": true,
		"target_visible": true
	}
	shoot.effects = {"target_dead": true}
	shoot.base_cost = 1
	actions.append(shoot)

	# Initial state: NPC has nothing
	var state := GOAPWorldState.new()
	state.set_fact("has_weapon", false)
	state.set_fact("weapon_loaded", false)
	state.set_fact("target_in_range", false)
	state.set_fact("target_visible", false)
	state.set_fact("target_dead", false)
	state.set_fact("near_weapon_pickup", false)
	state.set_fact("near_ammo", false)

	var goal := GOAPGoal.new()
	goal.target_state = {"target_dead": true}

	var plan := planner.plan(actions, state, goal)

	assert_true(plan.size() > 0, "found a plan")

	# Verify last action is Shoot
	if plan.size() > 0:
		assert_eq(plan[plan.size() - 1].action_name, "Shoot", "last action is Shoot")

	# Verify pickup comes before shoot
	var pickup_idx := -1
	var shoot_idx := -1
	for i in range(plan.size()):
		if plan[i].action_name == "PickUpWeapon":
			pickup_idx = i
		if plan[i].action_name == "Shoot":
			shoot_idx = i

	assert_true(pickup_idx >= 0, "plan includes PickUpWeapon")
	assert_true(shoot_idx >= 0, "plan includes Shoot")
	assert_true(pickup_idx < shoot_idx, "PickUpWeapon comes before Shoot")

	print("  Plan: " + " -> ".join(plan.map(func(a): return a.action_name)))
	print("  Planning time: %.3f ms" % planner.get_last_plan_time_ms())
	print("  Iterations: %d" % planner.get_last_iterations())
