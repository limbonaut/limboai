## Goal Evaluator
## Evaluates and switches between GOAP goals based on world state
## Priority: EvadeMelee (melee threat) > AvoidDamage (ranged threat) > RegainHealth (low health) > KillTarget (default)
## Distinguishes between melee and ranged threats for tactical response:
## - Melee threat → EvadeMelee (maintain distance, retreat)
## - Ranged threat → AvoidDamage (take cover)
class_name GoalEvaluator
extends Node

const GOAPUtilsClass = preload("res://demo/ai/goap/goap_utils.gd")

signal goal_changed(new_goal: Resource, goal_type: String)

@export var bt_player: BTPlayer
@export var goal_kill_target: Resource   # GOAPGoal - default aggressive goal
@export var goal_avoid_damage: Resource  # GOAPGoal - seek cover (for ranged threats)
@export var goal_regain_health: Resource # GOAPGoal - find health
@export var goal_evade_melee: Resource   # GOAPGoal - maintain distance (for melee threats)

var current_goal_type: String = "kill"

# Cached GOAP task reference
var _goap_task = null

# Threat state tracking
var _melee_threat := false
var _ranged_threat := false


func _ready() -> void:
	# Defer finding GOAP task until BT is initialized
	call_deferred("_find_and_cache_goap_task")


func _find_and_cache_goap_task() -> void:
	_goap_task = GOAPUtilsClass.find_goap_task_from_player(bt_player)


## Updates threat state - called by WorldStateManager signals
func set_melee_threat(is_threatened: bool) -> void:
	_melee_threat = is_threatened


func set_ranged_threat(is_threatened: bool) -> void:
	_ranged_threat = is_threatened


## Evaluates which goal should be active and switches if needed
## Returns true if goal was changed
## Now considers the TYPE of threat, not just presence of threat
func evaluate(is_threatened: bool, is_low_health: bool) -> bool:
	var new_goal_type: String
	var new_goal: Resource

	# Priority-based goal selection with threat type awareness
	# 1. Melee threat → Evade (retreat/maintain distance) - cover won't help!
	# 2. Ranged threat → Take cover - cover blocks bullets
	# 3. Low health → Seek healing
	# 4. Default → Attack
	if _melee_threat and goal_evade_melee:
		new_goal_type = "evade"
		new_goal = goal_evade_melee
	elif _ranged_threat:
		new_goal_type = "avoid"
		new_goal = goal_avoid_damage
	elif is_low_health:
		new_goal_type = "health"
		new_goal = goal_regain_health
	else:
		new_goal_type = "kill"
		new_goal = goal_kill_target

	if new_goal_type == current_goal_type:
		return false  # No change needed

	if not new_goal:
		push_warning("GOAP GoalEvaluator: Goal resource not set for type: " + new_goal_type)
		return false

	# Apply the goal change
	if _apply_goal(new_goal):
		current_goal_type = new_goal_type
		goal_changed.emit(new_goal, new_goal_type)
		print("GOAP: Switched goal to %s (melee_threat=%s, ranged_threat=%s)" % [new_goal.goal_name, _melee_threat, _ranged_threat])
		return true

	return false


## Forces a specific goal type
func force_goal(goal_type: String) -> bool:
	var new_goal: Resource
	match goal_type:
		"kill":
			new_goal = goal_kill_target
		"avoid":
			new_goal = goal_avoid_damage
		"health":
			new_goal = goal_regain_health
		"evade":
			new_goal = goal_evade_melee
		_:
			push_warning("GOAP GoalEvaluator: Unknown goal type: " + goal_type)
			return false

	if _apply_goal(new_goal):
		current_goal_type = goal_type
		goal_changed.emit(new_goal, goal_type)
		return true

	return false


func _apply_goal(new_goal: Resource) -> bool:
	# Re-find GOAP task if not cached
	if not _goap_task:
		_goap_task = GOAPUtilsClass.find_goap_task_from_player(bt_player)

	if not _goap_task:
		push_warning("GOAP GoalEvaluator: Could not find BTRunGOAPPlan task")
		return false

	_goap_task.set_goal(new_goal)
	_goap_task.interrupt()  # Force replan with new goal
	return true


## Returns the currently active goal resource
func get_current_goal() -> Resource:
	match current_goal_type:
		"kill":
			return goal_kill_target
		"avoid":
			return goal_avoid_damage
		"health":
			return goal_regain_health
		"evade":
			return goal_evade_melee
	return null
