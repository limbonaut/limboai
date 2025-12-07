@tool
extends BTAction
## Moves toward target to close the distance for melee combat.
## Used by melee weapon agents to get in range.
## Uses PositionEvaluator for optimized approach with weapon-based speed.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const PositionEvaluatorClass = preload("res://demo/ai/goap/components/position_evaluator.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Distance at which we consider "close enough"
@export var close_distance := 100.0

## Cached base weights for reset
var _base_weights: Dictionary = {}


func _generate_name() -> String:
	return "GOAPCloseGap  target: %s" % LimboUtility.decorate_var(target_var)


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()
	var distance: float = agent.global_position.distance_to(target_node.global_position)

	# Check if we're close enough
	if distance < close_distance:
		agent.velocity = Vector2.ZERO
		print("GOAP: CloseGap - in melee range!")
		return SUCCESS

	# Get speed from combat component (weapon-based)
	var speed := GOAPConfigClass.MELEE_MOVE_SPEED
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			speed = combat.get_move_speed()

	# Get evaluator for smart movement
	var evaluator: Node = _get_evaluator()
	var move_dir: Vector2

	if evaluator:
		# Build context from blackboard
		var context := PositionEvaluatorClass.build_context_from_blackboard(blackboard, agent)

		# Set weapon type for weapon-aware positioning (close gap = melee)
		evaluator.set_weapon_type(false)

		# Store and apply approach mode weights
		_store_base_weights(evaluator)
		_apply_approach_weights(evaluator)

		# Get best position
		var best_pos: Vector2 = evaluator.get_best_position(agent.global_position, context)

		# Restore weights
		_restore_base_weights(evaluator)

		# Calculate direction to best position
		move_dir = (best_pos - agent.global_position).normalized()
	else:
		# Fallback: direct approach
		move_dir = dir_to_target

	# Move toward target/best position
	agent.velocity = move_dir * speed

	# Update agent facing (always face target in approach)
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if dir_to_target.x > 0:
			root.scale.x = scale_magnitude
		else:
			root.scale.x = -scale_magnitude

	# Play run animation if available
	if agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"run"):
			anim.play(&"run")
		elif anim.has_animation(&"walk"):
			anim.play(&"walk")

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


func _apply_approach_weights(evaluator: Node) -> void:
	# Approach mode: invert threat distance (want to get closer), reduce strafe
	evaluator.weight_threat_distance *= -1.0  # Invert - want closer
	evaluator.weight_strafe_preference *= 0.3  # Reduce strafe for direct approach
