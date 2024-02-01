#*
#* face_target.gd
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
## FaceTarget

@export var target_var: String = "target"

# Display a customized name (requires @tool).
func _generate_name() -> String:
	return "FaceTarget " + LimboUtility.decorate_var(target_var)


# Called each time this task is ticked (aka executed).
func _tick(_delta: float) -> Status:
	var target: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target):
		return FAILURE
	var dir: float = target.global_position.x - agent.global_position.x
	agent.velocity = Vector2.ZERO
	agent.face_dir(dir)
	return SUCCESS
