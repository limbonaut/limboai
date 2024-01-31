#*
#* arrive_pos.gd
#* =============================================================================
#* Copyright 2021-2024 Serhii Snitsaruk
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
@tool
extends BTAction
## Arrive to a position, with a bias to horizontal movement.


@export var target_position_var := "target_position"
@export var speed_var := "speed"
@export var tolerance := 50.0


func _generate_name() -> String:
	return "Arrive  pos: %s" % [
		LimboUtility.decorate_var(target_position_var),
	]

func _tick(_delta: float) -> Status:
	var target_pos: Vector2 = blackboard.get_var(target_position_var, Vector2.ZERO)
	if target_pos.distance_to(agent.global_position) < tolerance:
		return SUCCESS

	var speed: float = blackboard.get_var(speed_var, 10.0)
	var dist: float = absf(agent.global_position.x - target_pos.x)
	var vertical_factor: float = remap(dist, 200.0, 500.0, 1.0, 0.0)
	vertical_factor = clampf(vertical_factor, 0.0, 1.0)
	var dir: Vector2 = agent.global_position.direction_to(target_pos)
	dir.y *= vertical_factor
	agent.velocity = dir.normalized() * speed
	agent.move_and_slide()
	return RUNNING
