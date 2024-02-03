@tool
extends BTAction
## SelectRandomNearbyPos: Select a position nearby within specified range.
## Returns SUCCESS.

## Maximum distance to the desired position.
@export var range_min: float = 300.0
@export var range_max: float = 500.0
@export var position_var: String = "pos"

# Display a customized name (requires @tool).
func _generate_name() -> String:
	return "SelectRandomNearbyPos  range: [%s, %s]  ➜%s" % [
		range_min, range_max,
		LimboUtility.decorate_var(position_var)]

# Called each time this task is ticked (aka executed).
func _tick(_delta: float) -> Status:
	var pos: Vector2
	var is_good_position: bool = false
	while not is_good_position:
		# Randomize until we find a good position (in other words, not outside the arena)
		var angle: float = randf() * TAU
		var rand_distance: float = randf_range(range_min, range_max)
		pos = agent.global_position + Vector2(sin(angle), cos(angle)) * rand_distance
		is_good_position = agent.is_good_position(pos)
	blackboard.set_var(position_var, pos)
	return SUCCESS
