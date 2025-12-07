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
## - Periodic replanning to prevent stuck states
## - No-plan detection with fallback to KillTarget
class_name GoalEvaluator
extends Node

const GOAPUtilsClass = preload("res://demo/ai/goap/goap_utils.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

signal goal_changed(new_goal: Resource, goal_type: String)

@export var bt_player: BTPlayer
@export var goal_kill_target: Resource   # GOAPGoal - default aggressive goal
@export var goal_avoid_damage: Resource  # GOAPGoal - seek cover (for ranged threats)
@export var goal_regain_health: Resource # GOAPGoal - find health
@export var goal_evade_melee: Resource   # GOAPGoal - maintain distance (for melee threats)
@export var goal_get_speed_boost: Resource # GOAPGoal - opportunistically get speed boost

var current_goal_type: String = "kill"

# Cached GOAP task reference
var _goap_task = null

# Agent reference for observing opponent
var agent: Node2D

# Threat state tracking
var _melee_threat := false
var _ranged_threat := false

# Hysteresis settings to prevent goal thrashing (using optimized config values)
var GOAL_SWITCH_COOLDOWN: float        # Minimum time between any goal switches
var DEFENSIVE_GOAL_COMMITMENT: float   # Minimum time to commit to defensive goals
var ATTACK_GOAL_COMMITMENT: float      # Shorter commitment for attack goal
var DEFENSIVE_TIMEOUT: float           # Max time in defensive mode before forcing attack

var _time_since_last_switch := 0.0
var _time_in_current_goal := 0.0

# Opponent behavior tracking
var _opponent_last_in_cover := false
var _opponent_last_has_weapon := false
var _opponent_last_health := 100
var _opponent_behavior_changed := false

# Periodic replanning
const REPLAN_INTERVAL := 2.0  # Force replan every N seconds to prevent stuck states
const NO_PLAN_CHECK_INTERVAL := 0.3  # How often to check plan status (not every frame!)
var _time_since_last_replan := 0.0
var _time_since_last_plan_check := 0.0
var _consecutive_no_plan_count := 0  # Track consecutive checks with no plan
const MAX_NO_PLAN_BEFORE_FALLBACK := 3  # Switch to fallback after N consecutive no-plans


func _ready() -> void:
	# Initialize hysteresis settings from optimized config
	GOAL_SWITCH_COOLDOWN = GOAPConfigClass.GOAL_SWITCH_COOLDOWN
	DEFENSIVE_GOAL_COMMITMENT = GOAPConfigClass.DEFENSIVE_GOAL_COMMITMENT
	ATTACK_GOAL_COMMITMENT = GOAPConfigClass.ATTACK_GOAL_COMMITMENT
	DEFENSIVE_TIMEOUT = GOAPConfigClass.DEFENSIVE_TIMEOUT

	# Defer finding GOAP task until BT is initialized
	call_deferred("_find_and_cache_goap_task")
	# Get agent reference
	agent = get_parent()


func _process(delta: float) -> void:
	_time_since_last_switch += delta
	_time_in_current_goal += delta
	_time_since_last_replan += delta
	_time_since_last_plan_check += delta

	# Don't process if agent is dead
	if agent and "health" in agent and agent.health <= 0:
		return

	# Observe opponent behavior
	_observe_opponent()

	# Check for no-plan state periodically (not every frame!)
	if _time_since_last_plan_check >= NO_PLAN_CHECK_INTERVAL:
		_time_since_last_plan_check = 0.0
		_check_plan_status()

	# Periodic replanning to prevent stuck states
	if _time_since_last_replan >= REPLAN_INTERVAL:
		_time_since_last_replan = 0.0
		force_replan()

	# Check for defensive timeout - force attack if stuck defending too long
	# Even if threatened, being stuck in pure defense forever is bad
	if current_goal_type in ["avoid", "evade"] and _time_in_current_goal > DEFENSIVE_TIMEOUT:
		# If no immediate threat, switch to attack
		if not _melee_threat and not _ranged_threat:
			print("GOAP: Defensive timeout - forcing attack mode (no threat)")
			force_goal("kill")
		# If threatened but stuck too long (2x timeout), force switch anyway
		elif _time_in_current_goal > DEFENSIVE_TIMEOUT * 2.0:
			print("GOAP: Extended defensive timeout - forcing attack mode despite threat")
			force_goal("kill")


## Checks if current goal has a valid plan, handles fallback if stuck
func _check_plan_status() -> void:
	if not _goap_task:
		_goap_task = GOAPUtilsClass.find_goap_task_from_player(bt_player)
	if not _goap_task:
		return

	var plan = _goap_task.get_current_plan()

	if plan.is_empty():
		_consecutive_no_plan_count += 1

		# If we've been stuck with no plan for too long, switch to fallback goal
		if _consecutive_no_plan_count >= MAX_NO_PLAN_BEFORE_FALLBACK:
			var goal = _goap_task.get_goal()
			var goal_name = goal.goal_name if goal else "?"
			print("GOAP: No plan for '%s' after %d attempts - switching to fallback" % [goal_name, _consecutive_no_plan_count])

			# Try to switch to a different goal based on priority
			# If current is speed_boost or health, try kill
			# If current is kill, try evade/avoid based on threat
			if current_goal_type in ["speed_boost", "health"]:
				force_goal("kill")
			elif current_goal_type == "kill":
				# Try defensive goal if threatened, otherwise just replan
				if _melee_threat and goal_evade_melee:
					force_goal("evade")
				elif _ranged_threat:
					force_goal("avoid")
				else:
					# Just force replan - maybe world state changed
					force_replan()
			elif current_goal_type in ["evade", "avoid"]:
				# Stuck in defensive mode with no plan - try attacking
				force_goal("kill")
			else:
				# Default fallback to kill
				force_goal("kill")

			_consecutive_no_plan_count = 0
	else:
		# We have a valid plan - reset counter
		_consecutive_no_plan_count = 0


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

	# Special case: if on speed_boost goal but we now have it, immediately switch to kill
	# This bypasses hysteresis since the goal is satisfied
	if current_goal_type == "speed_boost" and not _should_get_speed_boost():
		return force_goal("kill")

	# Priority-based goal selection with threat type awareness
	# 1. Melee threat → Evade (retreat/maintain distance) - cover won't help!
	# 2. Ranged threat → Take cover - cover blocks bullets
	# 3. Low health → Seek healing
	# 4. Default → Attack (but consider speed boost opportunity)
	if _melee_threat and goal_evade_melee:
		new_goal_type = "evade"
		new_goal = goal_evade_melee
	elif _ranged_threat:
		new_goal_type = "avoid"
		new_goal = goal_avoid_damage
	elif is_low_health:
		new_goal_type = "health"
		new_goal = goal_regain_health
	elif _should_get_speed_boost():
		new_goal_type = "speed_boost"
		new_goal = goal_get_speed_boost
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


## Checks if agent should opportunistically get speed boost
## Returns true when: not already boosted, speed boost available, not actively in danger,
## AND we're closer to the speed boost than the enemy (we can reach it first)
func _should_get_speed_boost() -> bool:
	if not goal_get_speed_boost:
		return false
	if not agent:
		return false

	# Don't pursue speed boost if we're actively threatened
	if _melee_threat or _ranged_threat:
		return false

	# Check if agent already has speed boost
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if "is_speed_boosted" in combat and combat.is_speed_boosted:
			return false

	# Check if speed boost pickup is available via blackboard
	var bb := bt_player.get_blackboard() if bt_player else null
	if not bb:
		return false

	var speed_boost_available: bool = bb.get_var(&"speed_boost_available", false)
	var has_speed_boost: bool = bb.get_var(&"has_speed_boost", false)
	if not speed_boost_available or has_speed_boost:
		return false

	# Simple safety check: Only pursue if we're closer to the pickup than the enemy
	# This ensures we can reach it first without getting intercepted
	var target: Node2D = bb.get_var(&"target", null)
	var speed_boost_pickup: Node2D = bb.get_var(&"speed_boost_pickup", null)
	if is_instance_valid(target) and is_instance_valid(speed_boost_pickup):
		var agent_pos: Vector2 = agent.global_position
		var target_pos: Vector2 = target.global_position
		var pickup_pos: Vector2 = speed_boost_pickup.global_position

		var our_dist: float = agent_pos.distance_to(pickup_pos)
		var enemy_dist: float = target_pos.distance_to(pickup_pos)

		# Only pursue if we're significantly closer (at least 100 units advantage)
		# This gives us margin to reach it safely
		if our_dist > enemy_dist - 100.0:
			return false

	return true


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
		"speed_boost":
			new_goal = goal_get_speed_boost
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
		"speed_boost":
			return goal_get_speed_boost
	return null


## Forces an immediate replan without changing the goal
## Used when world state changes significantly (e.g., weapon pickup)
func force_replan() -> void:
	if not _goap_task:
		_goap_task = GOAPUtilsClass.find_goap_task_from_player(bt_player)

	if _goap_task:
		_goap_task.interrupt()
		print("GOAP GoalEvaluator: Forced replan")
