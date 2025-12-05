@tool
extends BTAction
## Retreats away from target while keeping them in sight.
## Used by ranged weapon agents to maintain distance.
## Strafes backward while facing the target.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Retreat speed (backwards strafe)
@export var retreat_speed := 250.0

## Safe distance from target
@export var safe_distance := 300.0


func _generate_name() -> String:
	return "GOAPRetreat  target: %s" % LimboUtility.decorate_var(target_var)


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()
	var distance: float = agent.global_position.distance_to(target_node.global_position)

	# Check if we've retreated far enough
	if distance >= safe_distance:
		agent.velocity = Vector2.ZERO
		print("GOAP: Retreat - safe distance achieved!")
		return SUCCESS

	# Move away from target (opposite direction)
	var retreat_dir: Vector2 = -dir_to_target

	# Check arena bounds and adjust if needed
	var next_pos: Vector2 = agent.global_position + retreat_dir * retreat_speed * delta
	if next_pos.x < GOAPConfigClass.ARENA_MIN.x or next_pos.x > GOAPConfigClass.ARENA_MAX.x:
		# Can't retreat further horizontally, try diagonal
		retreat_dir.x = 0.0
		retreat_dir = retreat_dir.normalized() if retreat_dir.length() > 0 else Vector2.UP
	if next_pos.y < GOAPConfigClass.ARENA_MIN.y or next_pos.y > GOAPConfigClass.ARENA_MAX.y:
		retreat_dir.y = 0.0
		retreat_dir = retreat_dir.normalized() if retreat_dir.length() > 0 else Vector2.RIGHT

	# If we can't retreat anywhere, just stop
	if retreat_dir.length() < 0.1:
		agent.velocity = Vector2.ZERO
		print("GOAP: Retreat - cornered, can't retreat further!")
		return SUCCESS

	agent.velocity = retreat_dir * retreat_speed

	# Keep facing the target while retreating
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir_to_target.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0

	# Play walk animation
	if agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"walk"):
			anim.play(&"walk")

	return RUNNING
