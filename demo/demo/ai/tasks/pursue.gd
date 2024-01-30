@tool
extends BTAction

const TOLERANCE := 30.0

@export var target_var: String = "target"
@export var speed_var: String = "speed"
@export var distance: float = 100.0

func _generate_name() -> String:
	return "Pursue %s" % [LimboUtility.decorate_var(target_var)]


func _tick(_delta: float) -> Status:
	var target := blackboard.get_var(target_var, null) as CharacterBody2D
	if not is_instance_valid(target):
		return FAILURE

	var dir: Vector2 = target.global_position - agent.global_position
	var target_pos := Vector2(
		target.global_position.x - distance * signf(dir.x),
		target.global_position.y)

	if agent.global_position.distance_to(target_pos) < TOLERANCE:
		return SUCCESS

	var speed: float = blackboard.get_var(speed_var, 200.0)
	agent.velocity = agent.global_position.direction_to(target_pos) * speed
	agent.move_and_slide()
	return RUNNING
