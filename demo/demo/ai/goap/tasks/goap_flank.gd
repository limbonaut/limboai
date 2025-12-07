@tool
extends BTAction
## Moves the GOAP agent to a flanking position to get around enemy cover.
## Uses PositionEvaluator for optimized movement with strafe emphasis.
## Returns SUCCESS when flank position reached, RUNNING while moving.

const PositionEvaluatorClass = preload("res://demo/ai/goap/components/position_evaluator.gd")
const ArenaUtilityClass = preload("res://demo/ai/goap/arena_utility.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

## Blackboard variable that stores the target node (Node2D)
@export var target_var := &"target"

## How close should the agent be to the flank position to return SUCCESS
@export var tolerance := 50.0

## Movement speed
@export var speed := 280.0

## How far to flank (perpendicular distance from line to target)
@export var flank_distance := 200.0

var _flank_position: Vector2 = Vector2.ZERO
var _flank_direction: int = 0  # -1 for left, 1 for right

## Cached base weights for reset
var _base_weights: Dictionary = {}


func _generate_name() -> String:
	return "GOAPFlank  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_flank_position = Vector2.ZERO
	_flank_direction = 0
	print("GOAP ACTION: Flank started")


func _tick(_delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		print("GOAP ACTION: Flank - target invalid!")
		return FAILURE

	# Calculate flank position on first tick or if target moved significantly
	if _flank_position == Vector2.ZERO or _flank_direction == 0:
		_calculate_flank_position(target_node)

	var distance: float = agent.global_position.distance_to(_flank_position)

	if distance < tolerance:
		agent.velocity = Vector2.ZERO
		print("GOAP ACTION: Flank - reached flank position, target should be visible")
		return SUCCESS

	# Get evaluator for smart movement
	var evaluator: Node = _get_evaluator()
	var move_dir: Vector2

	if evaluator:
		# Build context from blackboard
		var context := PositionEvaluatorClass.build_context_from_blackboard(blackboard, agent)

		# Set weapon type for weapon-aware positioning
		var is_ranged: bool = context.get("is_ranged_weapon", false)
		evaluator.set_weapon_type(is_ranged)

		# Store and apply flank mode weights
		_store_base_weights(evaluator)
		_apply_flank_weights(evaluator)

		# Get best position
		var best_pos: Vector2 = evaluator.get_best_position(agent.global_position, context)

		# Restore weights
		_restore_base_weights(evaluator)

		# Calculate direction to best position
		move_dir = (best_pos - agent.global_position).normalized()
	else:
		# Fallback: direct approach to flank position
		move_dir = agent.global_position.direction_to(_flank_position)

	agent.velocity = move_dir * speed

	# Update facing toward target
	var face_dir: Vector2 = agent.global_position.direction_to(target_node.global_position)
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if face_dir.x > 0.1 and root.scale.x < 0:
			root.scale.x = scale_magnitude
		elif face_dir.x < -0.1 and root.scale.x > 0:
			root.scale.x = -scale_magnitude

	# Play walk animation
	if "animation_player" in agent and agent.animation_player:
		agent.animation_player.play(&"walk")
	elif agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"walk"):
			anim.play(&"walk")

	return RUNNING


func _calculate_flank_position(target_node: Node2D) -> void:
	var to_target: Vector2 = target_node.global_position - agent.global_position

	# Get perpendicular direction (both options)
	var perp_right: Vector2 = Vector2(-to_target.y, to_target.x).normalized()
	var perp_left: Vector2 = Vector2(to_target.y, -to_target.x).normalized()

	# Calculate both possible flank positions
	var mid_point: Vector2 = agent.global_position + to_target * 0.5
	var flank_right: Vector2 = mid_point + perp_right * flank_distance
	var flank_left: Vector2 = mid_point + perp_left * flank_distance

	# Choose the flank direction that keeps us more in bounds and closer to current position
	# Prefer the side we're already slightly on
	var current_side: float = signf(perp_right.dot(agent.global_position - mid_point))

	if current_side >= 0:
		_flank_position = flank_right
		_flank_direction = 1
	else:
		_flank_position = flank_left
		_flank_direction = -1

	# Clamp to arena bounds
	var margin := 50.0
	_flank_position.x = clampf(_flank_position.x, GOAPConfigClass.ARENA_MIN.x + margin, GOAPConfigClass.ARENA_MAX.x - margin)
	_flank_position.y = clampf(_flank_position.y, GOAPConfigClass.ARENA_MIN.y + margin, GOAPConfigClass.ARENA_MAX.y - margin)

	print("GOAP ACTION: Flank - calculated flank position at %s (direction: %d)" % [_flank_position, _flank_direction])


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


func _apply_flank_weights(evaluator: Node) -> void:
	# Flank mode: emphasize strafe movement, moderate threat distance
	evaluator.weight_strafe_preference *= 2.0
	evaluator.weight_threat_distance *= 0.5  # Stay at moderate distance
