## Goal Evaluator
## Evaluates and switches between GOAP goals based on world state
## Priority: EvadeMelee (melee threat) > AvoidDamage (ranged threat) > RegainHealth (low health) > KillTarget (default)
## Distinguishes between melee and ranged threats for tactical response:
## - Melee threat → EvadeMelee (maintain distance, retreat)
## - Ranged threat → AvoidDamage (take cover)
##
## Features:
## - Hysteresis to prevent goal thrashing
## - Opponent behavior observation - reacts to enemy state changes
## - Defensive timeout - returns to attack if stuck defending too long
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

# Agent reference for observing opponent
var agent: Node2D

# Threat state tracking
var _melee_threat := false
var _ranged_threat := false

# Hysteresis settings to prevent goal thrashing
const GOAL_SWITCH_COOLDOWN := 0.5        # Minimum time between any goal switches
const DEFENSIVE_GOAL_COMMITMENT := 1.0   # Minimum time to commit to defensive goals
const ATTACK_GOAL_COMMITMENT := 0.3      # Shorter commitment for attack goal
const DEFENSIVE_TIMEOUT := 4.0           # Max time in defensive mode before forcing attack

var _time_since_last_switch := 0.0
var _time_in_current_goal := 0.0

# Opponent behavior tracking
var _opponent_last_in_cover := false
var _opponent_last_has_weapon := false
var _opponent_last_health := 100
var _opponent_behavior_changed := false


func _ready() -> void:
	# Defer finding GOAP task until BT is initialized
	call_deferred("_find_and_cache_goap_task")
	# Get agent reference
	agent = get_parent()


func _process(delta: float) -> void:
	_time_since_last_switch += delta
	_time_in_current_goal += delta

	# Observe opponent behavior
	_observe_opponent()

	# Check for defensive timeout - force attack if stuck defending too long
	if current_goal_type in ["avoid", "evade"] and _time_in_current_goal > DEFENSIVE_TIMEOUT:
		if not _melee_threat and not _ranged_threat:
			print("GOAP: Defensive timeout - forcing attack mode")
			force_goal("kill")


## Observes opponent state and flags when behavior changes
func _observe_opponent() -> void:
	if not agent or not "target" in agent:
		return

	var target: Node2D = agent.target
	if not is_instance_valid(target):
		return

	# Track opponent state changes
	var opponent_in_cover: bool = target.in_cover if "in_cover" in target else false
	var opponent_has_weapon: bool = target.has_weapon if "has_weapon" in target else false
	var opponent_health: int = target.health if "health" in target else 100

	# Detect behavior changes
	if opponent_in_cover != _opponent_last_in_cover:
		_on_opponent_cover_changed(opponent_in_cover)
		_opponent_last_in_cover = opponent_in_cover

	if opponent_has_weapon != _opponent_last_has_weapon:
		_on_opponent_weapon_changed(opponent_has_weapon)
		_opponent_last_has_weapon = opponent_has_weapon

	# Detect significant health drop (opponent took damage)
	if opponent_health < _opponent_last_health - 10:
		_on_opponent_took_damage(_opponent_last_health - opponent_health)
	_opponent_last_health = opponent_health


## Called when opponent enters or leaves cover
func _on_opponent_cover_changed(is_in_cover: bool) -> void:
	if is_in_cover:
		# Opponent hiding - opportunity to approach or flank!
		print("GOAP: Opponent entered cover - considering aggressive response")
		if current_goal_type in ["avoid", "evade", "health"]:
			# If we're in defensive mode but opponent is hiding, we can attack
			if _time_in_current_goal > GOAL_SWITCH_COOLDOWN:
				_opponent_behavior_changed = true
				# Re-evaluate with bias toward attack
				_try_switch_to_attack("opponent hiding")
	else:
		# Opponent left cover - might be attacking
		print("GOAP: Opponent left cover - staying alert")


## Called when opponent picks up or loses weapon
func _on_opponent_weapon_changed(has_weapon: bool) -> void:
	if has_weapon:
		print("GOAP: Opponent armed - re-evaluating threat")
		# Opponent just got a weapon - this is a threat change, allow immediate re-eval
		_time_since_last_switch = GOAL_SWITCH_COOLDOWN  # Allow immediate switch
	else:
		print("GOAP: Opponent disarmed - opportunity to attack!")
		_try_switch_to_attack("opponent disarmed")


## Called when opponent takes significant damage
func _on_opponent_took_damage(amount: int) -> void:
	print("GOAP: Opponent took %d damage - pressing advantage" % amount)
	# Opponent is weakened - good time to be aggressive
	if current_goal_type != "kill" and not _melee_threat and not _ranged_threat:
		_try_switch_to_attack("opponent wounded")


## Attempts to switch to attack goal if conditions allow
func _try_switch_to_attack(reason: String) -> void:
	if current_goal_type == "kill":
		return  # Already attacking

	# Check if we can switch (respect minimal cooldown)
	if _time_since_last_switch < GOAL_SWITCH_COOLDOWN:
		return

	# Don't switch if there's an active threat
	if _melee_threat or _ranged_threat:
		return

	# Switch to attack
	print("GOAP: Switching to attack - %s" % reason)
	force_goal("kill")


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
## Includes hysteresis to prevent goal thrashing
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

	# === HYSTERESIS CHECK ===
	# Prevent rapid goal switching by enforcing minimum times

	# Always respect the cooldown between switches
	if _time_since_last_switch < GOAL_SWITCH_COOLDOWN:
		return false

	# Check goal commitment times based on current goal type
	var min_commitment := ATTACK_GOAL_COMMITMENT
	if current_goal_type in ["avoid", "evade", "health"]:
		min_commitment = DEFENSIVE_GOAL_COMMITMENT

	# Exception: Always allow switching TO a higher priority defensive goal
	var priority_upgrade := false
	if new_goal_type == "evade" and current_goal_type != "evade":
		priority_upgrade = true  # Melee threat is always urgent
	elif new_goal_type == "avoid" and current_goal_type in ["health", "kill"]:
		priority_upgrade = true  # Ranged threat upgrades from health/kill

	# If not a priority upgrade, enforce commitment time
	if not priority_upgrade and _time_in_current_goal < min_commitment:
		return false

	# Apply the goal change
	if _apply_goal(new_goal):
		current_goal_type = new_goal_type
		_time_since_last_switch = 0.0
		_time_in_current_goal = 0.0
		goal_changed.emit(new_goal, new_goal_type)
		print("GOAP: Switched goal to %s (melee_threat=%s, ranged_threat=%s)" % [new_goal.goal_name, _melee_threat, _ranged_threat])
		return true

	return false


## Forces a specific goal type (bypasses hysteresis)
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
		_time_since_last_switch = 0.0
		_time_in_current_goal = 0.0
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


## Forces an immediate replan without changing the goal
## Used when world state changes significantly (e.g., weapon pickup)
func force_replan() -> void:
	if not _goap_task:
		_goap_task = GOAPUtilsClass.find_goap_task_from_player(bt_player)

	if _goap_task:
		_goap_task.interrupt()
		print("GOAP GoalEvaluator: Forced replan")
