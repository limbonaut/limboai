@tool
extends BTAction
## SelectRandomNearbyPos: Select a position nearby within specified range.
## Returns SUCCESS.

## Maximum distance to the desired position.
@export var range_min: float = 300.0
@export var range_max: float = 500.0
@export var position_var: String = "_pos"

# Display a customized name (requires @tool).
func _generate_name() -> String:
	return "SelectRandomNearbyPos  range: [%s, %s]  ➜%s" % [
		range_min, range_max,
		LimboUtility.decorate_var(position_var)]

# Called each time this task is ticked (aka executed).
func _tick(_delta: float) -> Status:
	var angle: float = randf() * TAU
	var rand_distance: float = randf_range(range_min, range_max)
	var pos: Vector2 = agent.global_position + Vector2(sin(angle), cos(angle)) * rand_distance
	blackboard.set_var(position_var, pos)
	return SUCCESS
