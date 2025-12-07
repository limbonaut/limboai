@tool
extends BTAction
## Unified Smart Movement using Position Evaluator.
## Replaces specialized movement tasks with a single optimizable system.
##
## Movement mode determines weight multipliers:
## - RETREAT: Prioritize distance from threat
## - KITE: Maintain distance while allowing shooting
## - APPROACH: Close distance to target
## - SEEK_AMMO: Move toward ammo pickup
## - SEEK_HEALTH: Move toward health pickup
## - SEEK_COVER: Move toward cover
## - FLANK: Circle around to side of target
## - EVADE: Emergency escape (maximum threat avoidance)

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const PositionEvaluatorClass = preload("res://demo/ai/goap/components/position_evaluator.gd")

enum MoveMode {
	RETREAT,      ## Prioritize distance from threat
	KITE,         ## Maintain distance while shooting
	APPROACH,     ## Close distance to target
	SEEK_AMMO,    ## Move toward ammo pickup
	SEEK_HEALTH,  ## Move toward health pickup
	SEEK_COVER,   ## Move toward cover
	FLANK,        ## Circle around to side
	EVADE,        ## Emergency escape
}

## Movement behavior mode
@export var mode: MoveMode = MoveMode.RETREAT

## Blackboard variable storing the target/threat node
@export var target_var := &"target"

## Success condition: distance from threat (for RETREAT/KITE/EVADE)
@export var safe_distance := 300.0

## Success condition: distance to target (for APPROACH)
@export var close_distance := 100.0

## Success condition: distance to resource (for SEEK_* modes)
@export var pickup_distance := 80.0

## Optional: specific resource variable for SEEK modes
@export var resource_var := &""

## Cached base weights for reset
var _base_weights: Dictionary = {}


func _generate_name() -> String:
	var mode_name := MoveMode.keys()[mode]
	return "GOAPSmartMove [%s] target: %s" % [mode_name, LimboUtility.decorate_var(target_var)]


func _enter() -> void:
	var mode_name := MoveMode.keys()[mode]
	print("GOAP: SmartMove [%s] started" % mode_name)


func _tick(delta: float) -> Status:
	# Get position evaluator
	var evaluator: Node = _get_evaluator()
	if evaluator == null:
		push_warning("GOAP SmartMove: No PositionEvaluator found on agent!")
		return FAILURE

	# Build context from blackboard
	var context := PositionEvaluatorClass.build_context_from_blackboard(blackboard, agent)

	# Set weapon type for weapon-aware positioning
	var is_ranged: bool = context.get("is_ranged_weapon", false)
	evaluator.set_weapon_type(is_ranged)

	# Store base weights before applying mode multipliers
	_store_base_weights(evaluator)

	# Apply mode-specific weight adjustments
	_apply_mode_weights(evaluator)

	# Get best position
	var best_pos: Vector2 = evaluator.get_best_position(agent.global_position, context)

	# Reset weights after use
	_restore_base_weights(evaluator)

	# Check success conditions based on mode
	var success := _check_success_condition(context)
	if success:
		agent.velocity = Vector2.ZERO
		_play_idle_animation()
		return SUCCESS

	# Move toward best position
	var move_dir := (best_pos - agent.global_position).normalized()
	var speed := _get_move_speed()

	agent.velocity = move_dir * speed

	# Update facing based on mode
	_update_facing(move_dir, context)

	# Play walk animation
	_play_walk_animation()

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


func _apply_mode_weights(evaluator: Node) -> void:
	match mode:
		MoveMode.RETREAT:
			evaluator.weight_threat_distance *= evaluator.retreat_threat_mult
			evaluator.weight_strafe_preference *= 0.8
			evaluator.weight_center_proximity *= 1.2
		MoveMode.KITE:
			evaluator.weight_threat_distance *= 1.5
			evaluator.weight_strafe_preference *= evaluator.kite_strafe_mult
			evaluator.weight_ammo_proximity *= 1.5
		MoveMode.APPROACH:
			evaluator.weight_threat_distance *= -1.0  # Invert - want closer
			evaluator.weight_strafe_preference *= 0.3
		MoveMode.SEEK_AMMO:
			evaluator.weight_ammo_proximity *= 3.0
			evaluator.weight_threat_distance *= 0.5
		MoveMode.SEEK_HEALTH:
			evaluator.weight_health_proximity *= 3.0
			evaluator.weight_threat_distance *= 0.5
		MoveMode.SEEK_COVER:
			evaluator.weight_cover_proximity *= 3.0
			evaluator.weight_threat_distance *= 0.3
		MoveMode.FLANK:
			evaluator.weight_strafe_preference *= 2.0
			evaluator.weight_threat_distance *= 0.5
		MoveMode.EVADE:
			evaluator.weight_threat_distance *= 3.0
			evaluator.weight_center_proximity *= 0.5
			evaluator.weight_strafe_preference *= 0.5


func _check_success_condition(context: Dictionary) -> bool:
	var threat_pos: Vector2 = context.get("threat_pos", Vector2.ZERO)

	match mode:
		MoveMode.RETREAT, MoveMode.KITE, MoveMode.EVADE:
			if threat_pos == Vector2.ZERO:
				return true  # No threat, success
			var dist := agent.global_position.distance_to(threat_pos)
			return dist >= safe_distance

		MoveMode.APPROACH:
			if threat_pos == Vector2.ZERO:
				return false  # Need a target to approach
			var dist := agent.global_position.distance_to(threat_pos)
			return dist <= close_distance

		MoveMode.SEEK_AMMO:
			var ammo_pos: Vector2 = context.get("ammo_pos", Vector2.ZERO)
			if ammo_pos == Vector2.ZERO:
				return false
			return agent.global_position.distance_to(ammo_pos) <= pickup_distance

		MoveMode.SEEK_HEALTH:
			var health_pos: Vector2 = context.get("health_pos", Vector2.ZERO)
			if health_pos == Vector2.ZERO:
				return false
			return agent.global_position.distance_to(health_pos) <= pickup_distance

		MoveMode.SEEK_COVER:
			var cover_pos: Vector2 = context.get("cover_pos", Vector2.ZERO)
			if cover_pos == Vector2.ZERO:
				return false
			return agent.global_position.distance_to(cover_pos) <= pickup_distance

		MoveMode.FLANK:
			# Flank success: perpendicular to threat and at moderate distance
			if threat_pos == Vector2.ZERO:
				return false
			var to_threat := (threat_pos - agent.global_position).normalized()
			var dist := agent.global_position.distance_to(threat_pos)
			# Consider success if we're at a reasonable flank distance
			return dist >= safe_distance * 0.5 and dist <= safe_distance * 1.5

	return false


func _get_move_speed() -> float:
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			return combat.get_move_speed()

	# Fallback to config defaults
	return GOAPConfigClass.MOVE_SPEED


func _update_facing(move_dir: Vector2, context: Dictionary) -> void:
	var face_dir := move_dir

	# In certain modes, face the threat instead of movement direction
	match mode:
		MoveMode.RETREAT, MoveMode.KITE, MoveMode.EVADE, MoveMode.FLANK:
			var threat_pos: Vector2 = context.get("threat_pos", Vector2.ZERO)
			if threat_pos != Vector2.ZERO:
				face_dir = (threat_pos - agent.global_position).normalized()

	# Apply facing
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if face_dir.x > 0.1:
			root.scale.x = scale_magnitude
		elif face_dir.x < -0.1:
			root.scale.x = -scale_magnitude


func _play_walk_animation() -> void:
	if agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"walk"):
			anim.play(&"walk")
	elif "animation_player" in agent and agent.animation_player:
		agent.animation_player.play(&"walk")


func _play_idle_animation() -> void:
	if agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"idle"):
			anim.play(&"idle")
	elif "animation_player" in agent and agent.animation_player:
		if agent.animation_player.has_animation(&"idle"):
			agent.animation_player.play(&"idle")
