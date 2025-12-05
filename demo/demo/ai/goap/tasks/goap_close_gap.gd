@tool
extends BTAction
## Moves toward target to close the distance for melee combat.
## Used by melee weapon agents to get in range.
## Uses weapon-based speed from CombatComponent.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

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

	# Get speed from combat component (weapon-based)
	var speed := GOAPConfigClass.MELEE_MOVE_SPEED
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			speed = combat.get_move_speed()

	# Move toward target
	agent.velocity = dir_to_target * speed

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
