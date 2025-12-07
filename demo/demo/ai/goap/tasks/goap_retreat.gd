@tool
extends BTAction
## Tactical Retreat: Uses PositionEvaluator for optimized retreat behavior.
## Circles around melee threats while heading toward resources.
## Uses optimizable weights to determine best positions.
## NOW WITH SHOOTING: Opportunistically shoots while retreating if ammo available.
##
## Behavior:
## - Evaluates positions using PositionEvaluator with retreat mode weights
## - Very close (danger zone): Emphasizes threat distance
## - Medium range: Balances retreat with strafe/resources
## - Safe range: Success - can transition to next action
## - Shoots at target while retreating (if has ammo and ranged weapon)

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const PositionEvaluatorClass = preload("res://demo/ai/goap/components/position_evaluator.gd")

## Ninja star scene for shooting while retreating
const NINJA_STAR_SCENE = preload("res://demo/agents/ninja_star/ninja_star.tscn")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Blackboard variable storing ammo pickup (preferred destination while evading)
@export var ammo_var := &"ammo_pickup"

## Fallback retreat speed (uses combat component speed when available)
@export var retreat_speed := 300.0

## Safe distance - we've evaded successfully (uses evaluator's safe_distance if available)
@export var safe_distance := 350.0

## Cooldown between shots while retreating
@export var shot_cooldown := 0.5

## Cached base weights for reset
var _base_weights: Dictionary = {}

## Shooting cooldown timer
var _cooldown_timer := 0.0


func _generate_name() -> String:
	return "GOAPRetreat  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	# Reset cooldown timer on entry
	_cooldown_timer = 0.0


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	var to_target: Vector2 = target_node.global_position - agent.global_position
	var distance: float = to_target.length()
	var dir_to_target: Vector2 = to_target.normalized()

	# Get evaluator for smart movement
	var evaluator: Node = _get_evaluator()

	# Check if we've retreated far enough (use evaluator's safe_distance if available)
	var safe_dist := safe_distance
	if evaluator:
		safe_dist = evaluator.safe_distance
	if distance >= safe_dist:
		agent.velocity = Vector2.ZERO
		return SUCCESS

	# Get speed from combat component (ranged is faster for kiting)
	var speed := retreat_speed
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			speed = combat.get_move_speed()

	# Calculate movement using PositionEvaluator if available
	var move_dir: Vector2
	if evaluator:
		# Build context from blackboard
		var context := PositionEvaluatorClass.build_context_from_blackboard(blackboard, agent)

		# Set weapon type for weapon-aware positioning
		var is_ranged: bool = context.get("is_ranged_weapon", false)
		evaluator.set_weapon_type(is_ranged)

		# Store and apply retreat mode weights
		_store_base_weights(evaluator)
		_apply_retreat_weights(evaluator)

		# Get best position
		var best_pos: Vector2 = evaluator.get_best_position(agent.global_position, context)

		# Restore weights
		_restore_base_weights(evaluator)

		# Calculate direction to best position
		move_dir = (best_pos - agent.global_position).normalized()
	else:
		# Fallback: simple retreat direction
		move_dir = -dir_to_target

	# If we can't move anywhere useful, just stop
	if move_dir.length() < 0.1:
		agent.velocity = Vector2.ZERO
		return SUCCESS

	agent.velocity = move_dir * speed

	# Keep facing the target while retreating/circling
	_update_facing(dir_to_target)

	# Play walk animation
	if agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"walk"):
			anim.play(&"walk")

	# Opportunistic shooting while retreating (if has ranged weapon and ammo)
	_cooldown_timer -= delta
	if _cooldown_timer <= 0.0:
		if _try_shoot_at_target(target_node, dir_to_target):
			_cooldown_timer = shot_cooldown

	return RUNNING


func _get_evaluator() -> Node:
	if agent.has_node("PositionEvaluator"):
		return agent.get_node("PositionEvaluator")
	return null


func _store_base_weights(evaluator: Node) -> void:
	_base_weights = {
		"threat": evaluator.weight_threat_distance,
		"ammo": evaluator.weight_ammo_proximity,
		"health": evaluator.weight_health_proximity,
		"cover": evaluator.weight_cover_proximity,
		"center": evaluator.weight_center_proximity,
		"strafe": evaluator.weight_strafe_preference,
		"speed_boost": evaluator.weight_speed_boost_proximity,
	}


func _restore_base_weights(evaluator: Node) -> void:
	if _base_weights.is_empty():
		return
	evaluator.weight_threat_distance = _base_weights.get("threat", 1.0)
	evaluator.weight_ammo_proximity = _base_weights.get("ammo", 0.5)
	evaluator.weight_health_proximity = _base_weights.get("health", 0.3)
	evaluator.weight_cover_proximity = _base_weights.get("cover", 0.4)
	evaluator.weight_center_proximity = _base_weights.get("center", 0.2)
	evaluator.weight_strafe_preference = _base_weights.get("strafe", 0.6)
	evaluator.weight_speed_boost_proximity = _base_weights.get("speed_boost", 0.2)


func _apply_retreat_weights(evaluator: Node) -> void:
	# Retreat mode: prioritize distance from threat based on how close enemy is
	# The key insight: when enemy is close, we must retreat FIRST, strafe SECOND
	var target_node: Node2D = blackboard.get_var(target_var)
	var distance: float = 999.0
	if is_instance_valid(target_node):
		distance = agent.global_position.distance_to(target_node.global_position)

	# Scale threat priority based on how close the enemy is
	if distance < evaluator.danger_distance:
		# DANGER ZONE: Pure retreat, no strafe
		evaluator.weight_threat_distance *= evaluator.retreat_threat_mult * 2.0
		evaluator.weight_strafe_preference *= 0.2  # Almost no strafe
		evaluator.weight_center_proximity *= 2.0   # Head toward center for escape options
	elif distance < evaluator.safe_distance:
		# CAUTION ZONE: Prioritize retreat, some strafe allowed
		evaluator.weight_threat_distance *= evaluator.retreat_threat_mult * 1.5
		evaluator.weight_strafe_preference *= 0.5
		evaluator.weight_center_proximity *= 1.5
	else:
		# SAFE ZONE: Normal retreat behavior
		evaluator.weight_threat_distance *= evaluator.retreat_threat_mult
		evaluator.weight_strafe_preference *= 0.8
		evaluator.weight_center_proximity *= 1.5  # Stronger center preference to avoid corners

	# If out of ammo, boost ammo priority significantly during retreat
	var ammo_ratio: float = 1.0
	if "ammo_count" in agent and "max_ammo" in agent and agent.max_ammo > 0:
		ammo_ratio = float(agent.ammo_count) / float(agent.max_ammo)
	if ammo_ratio < 0.2:  # Out of ammo or very low
		evaluator.weight_ammo_proximity *= 3.0  # Strong pull toward ammo


## Updates agent facing direction to always face the threat
func _update_facing(dir_to_target: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if dir_to_target.x > 0:
			root.scale.x = scale_magnitude
		else:
			root.scale.x = -scale_magnitude


## Attempts to shoot at target while retreating
## Returns true if shot was fired, false if couldn't shoot (no ammo, wrong weapon, etc.)
func _try_shoot_at_target(target_node: Node2D, dir_to_target: Vector2) -> bool:
	# Check if we have a ranged weapon
	if not agent.has_node("CombatComponent"):
		return false
	var combat = agent.get_node("CombatComponent")
	if not combat.is_ranged():
		return false  # Not a ranged weapon, can't shoot

	# Check ammo
	if agent.has_method("use_ammo"):
		if not agent.use_ammo():
			return false  # No ammo

	# Spawn ninja star
	var star = NINJA_STAR_SCENE.instantiate()
	var spawn_offset := dir_to_target * 60.0
	star.global_position = agent.global_position + spawn_offset + Vector2(0, -40)
	star.direction = dir_to_target
	star.shooter = agent
	agent.get_parent().add_child(star)

	# Apply suppression to target
	if target_node.has_node("CombatComponent"):
		var target_combat = target_node.get_node("CombatComponent")
		if target_combat.has_method("apply_suppression"):
			target_combat.apply_suppression()

	return true
