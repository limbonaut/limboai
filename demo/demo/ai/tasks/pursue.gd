#*
#* pursue.gd
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
## Pursue: Move towards target until agent is flanking it.
##
## Returns RUNNING, while moving towards target but not yet at the desired position.
## Returns SUCCESS, when at the desired position from target (flanking it).
## Returns FAILURE, if target is not a valid Node2D instance.


const TOLERANCE := 30.0

@export var target_var: String = "target"
@export var speed_var: String = "speed"
@export var approach_distance: float = 100.0

#var _side: float
var _waypoint: Vector2

# Display a customized name (requires @tool).
func _generate_name() -> String:
	return "Pursue %s" % [LimboUtility.decorate_var(target_var)]


func _enter() -> void:
	var target: Node2D = blackboard.get_var(target_var, null)
	if is_instance_valid(target):
		_select_new_waypoint(_get_desired_position(target))


# Called each time this task is ticked (aka executed).
func _tick(_delta: float) -> Status:
	var target: Node2D = blackboard.get_var(target_var, null)
	if not is_instance_valid(target):
		return FAILURE

	var desired_pos: Vector2 = _get_desired_position(target)
	if agent.global_position.distance_to(desired_pos) < TOLERANCE:
		return SUCCESS

	var speed: float = blackboard.get_var(speed_var, 200.0)
	if agent.global_position.distance_to(_waypoint) < TOLERANCE:
		_select_new_waypoint(desired_pos)
	var desired_velocity: Vector2 = agent.global_position.direction_to(_waypoint) * speed
	agent.velocity = lerp(agent.velocity, desired_velocity, 0.2)
	agent.move_and_slide()
	agent.update_facing()
	return RUNNING


## Get closest flanking position to target.
func _get_desired_position(target: Node2D) -> Vector2:
	var side: float = signf(agent.global_position.x - target.global_position.x)
	var desired_pos: Vector2 = target.global_position
	desired_pos.x += approach_distance * side
	return desired_pos


## Select an intermidiate waypoint towards the desired position.
func _select_new_waypoint(desired_position: Vector2) -> void:
	var distance_vector: Vector2 = desired_position - agent.global_position
	var angle_variation: float = randf_range(-0.2, 0.2)
	_waypoint = agent.global_position + distance_vector.limit_length(150.0).rotated(angle_variation)
