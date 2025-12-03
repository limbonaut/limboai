## GOAP Demo Setup Script
## This script demonstrates how to set up a GOAP-controlled NPC
## It creates all necessary actions, goals, and integrates with the behavior tree system
extends Node

## The Blackboard to use for world state
@export var blackboard: Blackboard

## Reference to the BTPlayer that will run the GOAP plan
@export var bt_player: BTPlayer

## Initialize the GOAP demo
func _ready() -> void:
	if blackboard == null:
		blackboard = Blackboard.new()

	# Initialize world state
	_setup_world_state()


## Set up the initial world state for the tactical NPC demo
func _setup_world_state() -> void:
	# Weapon state
	blackboard.set_var("has_weapon", false)
	blackboard.set_var("weapon_loaded", false)
	blackboard.set_var("near_weapon_pickup", false)
	blackboard.set_var("near_ammo", false)

	# Cover state
	blackboard.set_var("in_cover", false)
	blackboard.set_var("near_cover", false)

	# Target state
	blackboard.set_var("target_visible", false)
	blackboard.set_var("target_in_range", false)
	blackboard.set_var("target_in_cover", false)
	blackboard.set_var("target_dead", false)

	# NPC state
	blackboard.set_var("health_low", false)


## Create all GOAP actions for the tactical demo
## Returns an array of GOAPAction resources
static func create_tactical_actions() -> Array[GOAPAction]:
	var actions: Array[GOAPAction] = []

	# GoToWeapon - Move to weapon pickup location
	var go_to_weapon := GOAPAction.new()
	go_to_weapon.action_name = "GoToWeapon"
	go_to_weapon.preconditions = {}
	go_to_weapon.effects = {"near_weapon_pickup": true}
	go_to_weapon.base_cost = 3
	actions.append(go_to_weapon)

	# PickUpWeapon - Acquire a weapon
	var pickup_weapon := GOAPAction.new()
	pickup_weapon.action_name = "PickUpWeapon"
	pickup_weapon.preconditions = {"near_weapon_pickup": true}
	pickup_weapon.effects = {"has_weapon": true, "near_weapon_pickup": false}
	pickup_weapon.base_cost = 1
	actions.append(pickup_weapon)

	# GoToAmmo - Move to ammo location
	var go_to_ammo := GOAPAction.new()
	go_to_ammo.action_name = "GoToAmmo"
	go_to_ammo.preconditions = {"has_weapon": true}
	go_to_ammo.effects = {"near_ammo": true}
	go_to_ammo.base_cost = 3
	actions.append(go_to_ammo)

	# LoadWeapon - Load ammo into weapon
	var load_weapon := GOAPAction.new()
	load_weapon.action_name = "LoadWeapon"
	load_weapon.preconditions = {"has_weapon": true, "near_ammo": true}
	load_weapon.effects = {"weapon_loaded": true}
	load_weapon.base_cost = 1
	actions.append(load_weapon)

	# GoToCover - Move to cover position
	var go_to_cover := GOAPAction.new()
	go_to_cover.action_name = "GoToCover"
	go_to_cover.preconditions = {}
	go_to_cover.effects = {"near_cover": true}
	go_to_cover.base_cost = 2
	actions.append(go_to_cover)

	# TakeCover - Enter cover
	var take_cover := GOAPAction.new()
	take_cover.action_name = "TakeCover"
	take_cover.preconditions = {"near_cover": true}
	take_cover.effects = {"in_cover": true}
	take_cover.base_cost = 1
	actions.append(take_cover)

	# LeaveCover - Exit cover position
	var leave_cover := GOAPAction.new()
	leave_cover.action_name = "LeaveCover"
	leave_cover.preconditions = {"in_cover": true}
	leave_cover.effects = {"in_cover": false, "near_cover": false}
	leave_cover.base_cost = 1
	actions.append(leave_cover)

	# ApproachTarget - Move towards target
	var approach_target := GOAPAction.new()
	approach_target.action_name = "ApproachTarget"
	approach_target.preconditions = {"has_weapon": true}
	approach_target.effects = {"target_in_range": true, "target_visible": true}
	approach_target.base_cost = 4
	actions.append(approach_target)

	# Flank - Move to flanking position
	var flank := GOAPAction.new()
	flank.action_name = "Flank"
	flank.preconditions = {"target_in_cover": true, "has_weapon": true}
	flank.effects = {"target_visible": true, "target_in_cover": false}
	flank.base_cost = 5
	actions.append(flank)

	# Shoot - Fire weapon at target
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

	# MeleeAttack - Melee attack (expensive, last resort)
	var melee := GOAPAction.new()
	melee.action_name = "MeleeAttack"
	melee.preconditions = {"target_in_range": true, "target_visible": true}
	melee.effects = {"target_dead": true}
	melee.base_cost = 8
	actions.append(melee)

	# Retreat - Fall back when health is low
	var retreat := GOAPAction.new()
	retreat.action_name = "Retreat"
	retreat.preconditions = {"health_low": true}
	retreat.effects = {"in_cover": true, "near_cover": true, "target_in_range": false}
	retreat.base_cost = 3
	actions.append(retreat)

	return actions


## Create the primary goal: kill the target
static func create_kill_goal() -> GOAPGoal:
	var goal := GOAPGoal.new()
	goal.goal_name = "KillTarget"
	goal.target_state = {"target_dead": true}
	goal.priority = 10
	return goal


## Create the survival goal: get to cover when health is low
static func create_survival_goal() -> GOAPGoal:
	var goal := GOAPGoal.new()
	goal.goal_name = "Survive"
	goal.target_state = {"in_cover": true}
	goal.priority = 20  # Higher priority than kill
	return goal


## Utility function to print the current plan
static func print_plan(plan: Array) -> void:
	if plan.is_empty():
		print("No plan found!")
		return

	print("GOAP Plan (%d actions):" % plan.size())
	for i in range(plan.size()):
		var action: GOAPAction = plan[i]
		print("  %d. %s (cost: %d)" % [i + 1, action.action_name, action.base_cost])


## Demo function: run a planning scenario and print the result
func demo_planning() -> void:
	var planner := GOAPPlanner.new()
	var actions := create_tactical_actions()
	var goal := create_kill_goal()

	# Create world state from blackboard
	var world_state := GOAPWorldState.new()
	var fact_names: Array[StringName] = [
		"has_weapon", "weapon_loaded", "near_weapon_pickup", "near_ammo",
		"in_cover", "near_cover", "target_visible", "target_in_range",
		"target_in_cover", "target_dead", "health_low"
	]
	world_state.populate_from_blackboard(blackboard, fact_names)

	# Plan!
	var typed_actions: Array[GOAPAction] = []
	for action in actions:
		typed_actions.append(action)

	var plan := planner.plan(typed_actions, world_state, goal)

	print("\n=== GOAP Planning Demo ===")
	print("Goal: %s" % goal.goal_name)
	print("Planning took: %.3f ms" % planner.get_last_plan_time_ms())
	print("Iterations: %d" % planner.get_last_iterations())
	print_plan(plan)
	print("==========================\n")
