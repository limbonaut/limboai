@tool
extends BTAction
## Sprints toward target to close the distance for melee combat.
## Used by melee weapon agents to get in range.
## Faster than normal movement to show urgency.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Sprint speed (faster than normal move speed)
@export var sprint_speed := 450.0

## Distance at which we consider "close enough"
@export var close_distance := 100.0


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

	# Sprint toward target
	agent.velocity = dir_to_target * sprint_speed

	# Update agent facing
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir_to_target.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0

	# Play run animation if available
	if agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"run"):
			anim.play(&"run")
		elif anim.has_animation(&"walk"):
			anim.play(&"walk")

	return RUNNING
