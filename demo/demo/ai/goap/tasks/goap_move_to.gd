@tool
extends BTAction
## Moves the GOAP agent toward a target node stored in the blackboard.
## Returns SUCCESS when close to target, RUNNING while moving.

## Blackboard variable that stores the target node (Node2D)
@export var target_var := &"target"

## How close should the agent be to return SUCCESS
@export var tolerance := 80.0

## Movement speed
@export var speed := 300.0


func _generate_name() -> String:
	return "GOAPMoveTo  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	print("GOAP ACTION: MoveTo %s started" % target_var)


func _tick(_delta: float) -> Status:
	var target_value = blackboard.get_var(target_var)
	if target_value == null or not is_instance_valid(target_value):
		print("GOAP ACTION: MoveTo %s - target invalid!" % target_var)
		return FAILURE
	var target_node: Node2D = target_value

	# Special handling for cover objects - use get_cover_position to position behind cover
	var target_pos: Vector2
	if target_var == &"cover_object" and agent.has_method("get_cover_position"):
		target_pos = agent.get_cover_position()
	else:
		target_pos = target_node.global_position
	var distance: float = agent.global_position.distance_to(target_pos)

	if distance < tolerance:
		agent.velocity = Vector2.ZERO
		# If moving to cover, enter cover when we arrive
		if target_var == &"cover_object" and agent.has_method("enter_cover"):
			agent.enter_cover()
		print("GOAP ACTION: MoveTo %s - reached destination at %s (target was %s)" % [target_var, agent.global_position, target_pos])
		return SUCCESS

	var direction: Vector2 = agent.global_position.direction_to(target_pos)
	agent.velocity = direction * speed

	# Update facing
	if direction.x > 0.1 and agent.root.scale.x < 0:
		agent.root.scale.x = 1.0
	elif direction.x < -0.1 and agent.root.scale.x > 0:
		agent.root.scale.x = -1.0

	agent.animation_player.play(&"walk")
	return RUNNING
