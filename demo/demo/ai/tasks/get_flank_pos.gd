#*
#* GetFlankPos.gd
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
## GetFlankPos

@export var target_var: String = "_target"
@export var position_var: String = "flank_pos"
@export var distance: float = 300.0

# Display a customized name (requires @tool).
func _generate_name() -> String:
	return "GetFlankPos %s -> %s" % [
		LimboUtility.decorate_var(target_var),
		LimboUtility.decorate_var(position_var)]

# Called each time this task is ticked (aka executed).
func _tick(delta: float) -> Status:
	var target: CharacterBody2D = blackboard.get_var(target_var)
	if not is_instance_valid(target):
		return FAILURE
	var dir: float = signf(target.global_position.x - agent.global_position.x)
	var flank_pos: Vector2 = target.global_position
	flank_pos.x += dir * distance
	blackboard.set_var(position_var, flank_pos)
	return SUCCESS
