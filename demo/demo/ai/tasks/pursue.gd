@tool
extends BTAction
## Pursue target.
##
## Returns RUNNING, while moving towards target but not yet at the desired distance.
## Returns SUCCESS, when at the desired distance from target.
## Returns FAILURE, if target is not a valid instance.


const TOLERANCE := 30.0

@export var target_var: String = "target"
@export var speed_var: String = "speed"
@export var approach_distance: float = 100.0

var _side: float

# Display a customized name (requires @tool).
func _generate_name() -> String:
	return "Pursue %s" % [LimboUtility.decorate_var(target_var)]

# Called each time this task is entered.
func _enter() -> void:
	_side = 0.0

# Called each time this task is ticked (aka executed).
func _tick(_delta: float) -> Status:
	var target := blackboard.get_var(target_var, null) as CharacterBody2D
	if not is_instance_valid(target):
		return FAILURE

	if _side == 0:
		var dir: Vector2 = agent.global_position - target.global_position
		_side = signf(dir.x)
	var target_pos := Vector2(
		target.global_position.x + approach_distance * _side,
		target.global_position.y)
	if agent.global_position.distance_to(target_pos) < TOLERANCE:
		return SUCCESS

	var speed: float = blackboard.get_var(speed_var, 200.0)
	agent.velocity = agent.global_position.direction_to(target_pos) * speed
	agent.move_and_slide()
	return RUNNING
