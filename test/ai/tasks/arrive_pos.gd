@tool
@icon("res://icon.png")
class_name ArrivePos
extends BTAction

@export var target_position_var := "target_position"
@export var speed_var := "speed"
@export var tolerance := 50.0


func _generate_name() -> String:
	return "Arrive  pos: %s  speed: %s" % [
		LimboUtility.decorate_var(target_position_var),
		LimboUtility.decorate_var(speed_var),
	]

func _tick(p_delta: float) -> int:
	var target_pos: Vector2 = blackboard.get_var(target_position_var, Vector2.ZERO)
	if target_pos.distance_to(agent.global_position) < tolerance:
		return SUCCESS

	var speed: float = blackboard.get_var(speed_var, 10.0)
	var dir: Vector2 = agent.global_position.direction_to(target_pos)
	agent.global_position += dir * speed * p_delta
	return RUNNING
