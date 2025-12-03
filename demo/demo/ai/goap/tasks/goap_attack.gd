@tool
extends BTAction
## Throws a ninja star at the target as a ranged attack.
## Returns SUCCESS after firing, with a cooldown between shots.

## Blackboard variable storing the target node
@export var target_var := &"target"

## Cooldown between shots in seconds
@export var cooldown := 0.25

## Ninja star scene to spawn
const NINJA_STAR_SCENE = preload("res://demo/agents/ninja_star/ninja_star.tscn")

var _cooldown_timer := 0.0


func _generate_name() -> String:
	return "GOAPShoot  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_cooldown_timer = 0.0


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	# Wait for cooldown
	if _cooldown_timer > 0.0:
		_cooldown_timer -= delta
		return RUNNING

	# Consume ammo
	if agent.has_method("use_ammo"):
		if not agent.use_ammo():
			print("GOAP: No ammo for ranged attack!")
			return FAILURE

	# Reset cooldown for next shot
	_cooldown_timer = cooldown

	# Calculate direction to target
	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()

	# Update agent facing
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir_to_target.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0

	# Spawn ninja star at agent's position, traveling at an angle toward target
	var star = NINJA_STAR_SCENE.instantiate()
	star.global_position = agent.global_position
	star.direction = dir_to_target
	agent.get_parent().add_child(star)

	print("GOAP: Threw ninja star!")
	return RUNNING
