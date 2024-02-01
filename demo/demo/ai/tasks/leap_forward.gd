#*
#* leap_forward.gd
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
## LeapForward: Applies force each tick until duration is exceeded.
## Returns SUCCESS if elapsed time exceeded duration.
## Returns RUNNING if elapsed time didn't exceed duration.


@export var force: float = 1000.0
@export var duration: float = 0.1

# Display a customized name (requires @tool).
func _generate_name() -> String:
	return "LeapForward force: " + str(force)

# Called each time this task is ticked (aka executed).
func _tick(_delta: float) -> Status:
	var facing: float = agent.get_facing()
	var desired_velocity: Vector2 = Vector2.RIGHT * facing * force
	agent.velocity = lerp(agent.velocity, desired_velocity, 0.2)
	agent.move_and_slide()
	agent.update_facing()
	if elapsed_time > duration:
		return SUCCESS
	return RUNNING
