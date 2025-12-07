@tool
extends BTAction
## Kite Attack: Shoots while backing away from melee threats.
## Combines retreat movement with ranged shooting for optimal kiting behavior.
## Uses PositionEvaluator for optimized movement decisions.
## Returns RUNNING while kiting, SUCCESS when safe distance reached or target dead.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const PositionEvaluatorClass = preload("res://demo/ai/goap/components/position_evaluator.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Damage dealt by attack
@export var damage := 35

## Cooldown between shots in seconds
@export var shot_cooldown := 0.3

## Ninja star scene to spawn
const NINJA_STAR_SCENE = preload("res://demo/agents/ninja_star/ninja_star.tscn")

var _cooldown_timer := 0.0

## Cached base weights for reset
var _base_weights: Dictionary = {}


func _generate_name() -> String:
	return "GOAPKiteAttack  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_cooldown_timer = 0.0
	print("GOAP: Starting kite attack!")


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	# Check if target is dead
	if "health" in target_node and target_node.health <= 0:
		print("GOAP: Kite attack - target dead!")
		return SUCCESS

	# Check if we have ranged weapon and ammo
	if not agent.has_node("CombatComponent"):
		return FAILURE
	var combat = agent.get_node("CombatComponent")
	if not combat.is_ranged():
		print("GOAP: Kite attack requires ranged weapon!")
		return FAILURE

	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()
	var distance: float = agent.global_position.distance_to(target_node.global_position)

	# Get evaluator for smart movement
	var evaluator: Node = _get_evaluator()

	# Get safe distance for behavior decisions
	var safe_dist := GOAPConfigClass.RETREAT_DISTANCE
	if evaluator:
		safe_dist = evaluator.safe_distance

	# At safe distance: maintain position and keep shooting
	# Don't return SUCCESS - keep kiting until out of ammo or target dead
	var at_safe_distance := distance >= safe_dist

	# Get movement speed (ranged is faster)
	var speed: float = combat.get_move_speed()

	# Calculate movement using PositionEvaluator if available
	var move_dir: Vector2
	if at_safe_distance:
		# At safe distance: maintain position, maybe light strafe
		# Just stay still and shoot - let enemy come to us
		agent.velocity = Vector2.ZERO
		move_dir = Vector2.ZERO
	elif evaluator:
		# Build context from blackboard
		var context := PositionEvaluatorClass.build_context_from_blackboard(blackboard, agent)

		# Set weapon type for weapon-aware positioning (kite = always ranged)
		evaluator.set_weapon_type(true)

		# Store and apply kite mode weights
		_store_base_weights(evaluator)
		_apply_kite_weights(evaluator)

		# Get best position
		var best_pos: Vector2 = evaluator.get_best_position(agent.global_position, context)

		# Restore weights
		_restore_base_weights(evaluator)

		# Calculate direction to best position
		move_dir = (best_pos - agent.global_position).normalized()

		# Apply movement
		if move_dir.length() > 0.1:
			agent.velocity = move_dir * speed
		else:
			agent.velocity = Vector2.ZERO
	else:
		# Fallback: simple retreat direction
		move_dir = -dir_to_target
		agent.velocity = move_dir * speed

	# Keep facing the target while retreating
	_update_facing(dir_to_target)

	# Play appropriate animation
	if "animation_player" in agent:
		if at_safe_distance:
			agent.animation_player.play(&"idle")
		else:
			agent.animation_player.play(&"walk")

	# Handle shooting while retreating
	_cooldown_timer -= delta
	if _cooldown_timer <= 0.0:
		if _shoot_at_target(target_node, dir_to_target):
			_cooldown_timer = shot_cooldown
		else:
			# Out of ammo or jammed - return success to let GOAP replan
			print("GOAP: Kite attack - out of ammo!")
			return SUCCESS

	return RUNNING


func _shoot_at_target(target_node: Node2D, dir_to_target: Vector2) -> bool:
	# Consume ammo
	if agent.has_method("use_ammo"):
		if not agent.use_ammo():
			return false

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

	print("GOAP: Kite attack - threw ninja star!")
	return true


## Updates agent facing direction
func _update_facing(dir_to_target: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if dir_to_target.x > 0:
			root.scale.x = scale_magnitude
		else:
			root.scale.x = -scale_magnitude


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


func _apply_kite_weights(evaluator: Node) -> void:
	# Kite mode: HEAVILY prioritize distance from threat, reduce strafe when in danger
	# The key insight: when enemy is close, we must retreat FIRST, strafe SECOND
	var target_node: Node2D = blackboard.get_var(target_var)
	var distance: float = 999.0
	if is_instance_valid(target_node):
		distance = agent.global_position.distance_to(target_node.global_position)

	# Scale threat priority based on how close the enemy is
	# Close enemy = maximum retreat priority, minimal strafe
	# Far enemy = can afford to strafe more
	if distance < evaluator.danger_distance:
		# DANGER ZONE: Pure retreat, no strafe
		evaluator.weight_threat_distance *= 4.0
		evaluator.weight_strafe_preference *= 0.2  # Almost no strafe
		evaluator.weight_center_proximity *= 2.0   # Head toward center for escape options
	elif distance < evaluator.safe_distance:
		# CAUTION ZONE: Prioritize retreat, some strafe allowed
		evaluator.weight_threat_distance *= 2.5
		evaluator.weight_strafe_preference *= 0.6
		evaluator.weight_center_proximity *= 1.5
	else:
		# SAFE ZONE: Can strafe freely while maintaining distance
		evaluator.weight_threat_distance *= 1.5
		evaluator.weight_strafe_preference *= evaluator.kite_strafe_mult

	evaluator.weight_ammo_proximity *= 1.5
