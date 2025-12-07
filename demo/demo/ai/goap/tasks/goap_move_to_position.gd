@tool
extends BTAction
## Moves the GOAP agent toward a Vector2 position stored in the blackboard.
## Unlike GOAPMoveTo which requires a Node2D target, this accepts raw coordinates.
## Returns SUCCESS when close to target, RUNNING while moving.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const ArenaUtilityClass = preload("res://demo/ai/goap/arena_utility.gd")

## Blackboard variable that stores the target position (Vector2)
@export var position_var := &"target_position"

## How close should the agent be to return SUCCESS
@export var tolerance := 50.0

## Movement speed (uses CombatComponent speed if available)
@export var speed := 300.0

## Optional: face toward this target while moving (Node2D blackboard var)
@export var face_target_var := &""

## Clamp target position to arena bounds
@export var clamp_to_bounds := true


func _generate_name() -> String:
	return "GOAPMoveToPosition  pos: %s" % LimboUtility.decorate_var(position_var)


func _enter() -> void:
	print("GOAP ACTION: MoveToPosition %s started" % position_var)


func _tick(_delta: float) -> Status:
	var target_pos = blackboard.get_var(position_var)
	if target_pos == null or not target_pos is Vector2:
		print("GOAP ACTION: MoveToPosition - position invalid or not Vector2!")
		return FAILURE

	# Clamp target to arena bounds if enabled
	if clamp_to_bounds:
		target_pos = ArenaUtilityClass.clamp_to_arena(target_pos)

	var distance: float = agent.global_position.distance_to(target_pos)

	if distance < tolerance:
		agent.velocity = Vector2.ZERO
		print("GOAP ACTION: MoveToPosition - reached destination at %s (target was %s)" % [agent.global_position, target_pos])
		return SUCCESS

	# Get speed from combat component if available
	var move_speed := speed
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			move_speed = combat.get_move_speed()

	var direction: Vector2 = agent.global_position.direction_to(target_pos)
	agent.velocity = direction * move_speed

	# Update facing - either toward movement or toward a specific target
	var face_dir := direction
	if face_target_var != &"":
		var face_target: Node2D = blackboard.get_var(face_target_var, null)
		if is_instance_valid(face_target):
			face_dir = agent.global_position.direction_to(face_target.global_position)

	_update_facing(face_dir)

	# Play walk animation
	if agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"walk"):
			anim.play(&"walk")
	elif "animation_player" in agent and agent.animation_player:
		agent.animation_player.play(&"walk")

	return RUNNING


func _update_facing(dir: Vector2) -> void:
	# Try to access root node for facing
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if dir.x > 0.1 and root.scale.x < 0:
			root.scale.x = scale_magnitude
		elif dir.x < -0.1 and root.scale.x > 0:
			root.scale.x = -scale_magnitude
	elif "root" in agent and agent.root:
		var scale_magnitude := absf(agent.root.scale.x)
		if dir.x > 0.1 and agent.root.scale.x < 0:
			agent.root.scale.x = scale_magnitude
		elif dir.x < -0.1 and agent.root.scale.x > 0:
			agent.root.scale.x = -scale_magnitude
