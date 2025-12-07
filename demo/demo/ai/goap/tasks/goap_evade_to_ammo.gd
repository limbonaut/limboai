@tool
extends BTAction
## Evade To Ammo: Moves toward ammo pickup while evading melee threats.
## Uses PositionEvaluator for smart pathing that balances ammo seeking with threat avoidance.
## If melee gets too close, prioritizes evasion over ammo path.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const ArenaUtilityClass = preload("res://demo/ai/goap/arena_utility.gd")
const PositionEvaluatorClass = preload("res://demo/ai/goap/components/position_evaluator.gd")

## Blackboard variable storing the target (enemy)
@export var target_var := &"target"

## Blackboard variable storing the ammo pickup
@export var ammo_pickup_var := &"ammo_pickup"

## Tolerance for reaching ammo
@export var tolerance := 80.0

## Distance at which we prioritize evasion over ammo path
@export var danger_distance := 200.0

## Cached base weights for reset
var _base_weights: Dictionary = {}


func _generate_name() -> String:
	return "GOAPEvadeToAmmo"


func _enter() -> void:
	print("GOAP: Evading to ammo!")


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	var ammo_pickup: Node2D = blackboard.get_var(ammo_pickup_var)

	# Need ammo pickup to be valid
	if not is_instance_valid(ammo_pickup):
		print("GOAP: EvadeToAmmo - no ammo pickup!")
		return FAILURE

	# Check if ammo is available
	if ammo_pickup.has_method("is_available") and not ammo_pickup.is_available():
		print("GOAP: EvadeToAmmo - ammo not available!")
		return FAILURE

	var ammo_pos: Vector2 = ammo_pickup.global_position
	var distance_to_ammo: float = agent.global_position.distance_to(ammo_pos)

	# Check if we've reached the ammo
	if distance_to_ammo <= tolerance:
		agent.velocity = Vector2.ZERO
		# Actually pick up the ammo
		if agent.has_method("add_ammo"):
			agent.add_ammo(10)
		if ammo_pickup.has_method("collect"):
			ammo_pickup.collect()
		print("GOAP: EvadeToAmmo - collected ammo!")
		return SUCCESS

	# Get movement speed (ranged is faster)
	var speed: float = GOAPConfigClass.RANGED_MOVE_SPEED
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			speed = combat.get_move_speed()

	# Get direction to ammo for fallback and facing
	var dir_to_ammo: Vector2 = (ammo_pos - agent.global_position).normalized()
	var move_dir: Vector2 = dir_to_ammo

	# Try to use PositionEvaluator for smart movement
	var evaluator: Node = _get_evaluator()
	if evaluator:
		# Build context from blackboard
		var context := PositionEvaluatorClass.build_context_from_blackboard(blackboard, agent)

		# Set weapon type for weapon-aware positioning (evade to ammo = ranged)
		evaluator.set_weapon_type(true)

		# Store and apply evade-to-ammo weights
		_store_base_weights(evaluator)
		_apply_evade_ammo_weights(evaluator)

		# Get best position
		var best_pos: Vector2 = evaluator.get_best_position(agent.global_position, context)

		# Restore weights
		_restore_base_weights(evaluator)

		# Calculate direction to best position
		move_dir = (best_pos - agent.global_position).normalized()
	else:
		# Fallback: original manual calculation
		if is_instance_valid(target_node):
			var dir_to_threat: Vector2 = (target_node.global_position - agent.global_position).normalized()
			var distance_to_threat: float = agent.global_position.distance_to(target_node.global_position)
			var away_from_threat: Vector2 = -dir_to_threat

			var ammo_safety: float = dir_to_ammo.dot(away_from_threat)
			var safety_factor: float = (ammo_safety + 1.0) / 2.0
			var proximity_factor: float = clampf(1.0 - (distance_to_ammo / 400.0), 0.2, 1.0)
			var ammo_weight: float = clampf(safety_factor * 0.6 + proximity_factor * 0.4, 0.3, 0.8)

			if distance_to_threat < danger_distance:
				move_dir = (away_from_threat * (1.0 - ammo_weight * 0.5) + dir_to_ammo * ammo_weight * 0.5).normalized()
			else:
				move_dir = (away_from_threat * (1.0 - ammo_weight) + dir_to_ammo * ammo_weight).normalized()

	# Update facing - look at threat if present, otherwise look at ammo
	if is_instance_valid(target_node):
		var dir_to_threat: Vector2 = (target_node.global_position - agent.global_position).normalized()
		_update_facing(dir_to_threat)
	else:
		_update_facing(dir_to_ammo)

	# Apply movement
	agent.velocity = move_dir * speed

	# Play walk animation
	if "animation_player" in agent:
		agent.animation_player.play(&"walk")

	return RUNNING


## Updates agent facing direction
func _update_facing(dir: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if dir.x > 0:
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


func _apply_evade_ammo_weights(evaluator: Node) -> void:
	# Evade-to-ammo mode: balance ammo seeking with threat avoidance
	# Key insight: we need to path AROUND the threat, not through it!

	var target_node: Node2D = blackboard.get_var(target_var)
	var distance_to_threat: float = 999.0
	if is_instance_valid(target_node):
		distance_to_threat = agent.global_position.distance_to(target_node.global_position)

	# Scale weights based on how close the melee threat is
	if distance_to_threat < evaluator.danger_distance:
		# DANGER: Threat is close - prioritize evasion over ammo!
		evaluator.weight_threat_distance *= 3.5  # Strong evasion
		evaluator.weight_ammo_proximity *= 1.5   # Reduced ammo priority
		evaluator.weight_strafe_preference *= 2.0  # Heavy strafe to circle around
		evaluator.weight_center_proximity *= 2.0  # Stay in center for escape routes
	elif distance_to_threat < evaluator.safe_distance:
		# CAUTION: Threat approaching - balance evasion with ammo seeking
		evaluator.weight_threat_distance *= 2.5  # Good evasion
		evaluator.weight_ammo_proximity *= 2.5   # Moderate ammo pull
		evaluator.weight_strafe_preference *= 1.8  # Strafe around threat
		evaluator.weight_center_proximity *= 1.5  # Prefer center
	else:
		# SAFE: Can focus on ammo - threat is far
		evaluator.weight_ammo_proximity *= 4.0   # Strong ammo pull
		evaluator.weight_threat_distance *= 1.5  # Keep aware of threat
		evaluator.weight_strafe_preference *= 1.2  # Some strafe for safety
		evaluator.weight_center_proximity *= 1.2  # Slight center preference
