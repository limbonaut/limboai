@tool
extends BTAction
## Moves the GOAP agent to a flanking position to get around enemy cover.
## Calculates a position to the side of the target to gain line of sight.
## Returns SUCCESS when flank position reached, RUNNING while moving.

## Blackboard variable that stores the target node (Node2D)
@export var target_var := &"target"

## How close should the agent be to the flank position to return SUCCESS
@export var tolerance := 50.0

## Movement speed
@export var speed := 280.0

## How far to flank (perpendicular distance from line to target)
@export var flank_distance := 200.0

var _flank_position: Vector2 = Vector2.ZERO
var _flank_direction: int = 0  # -1 for left, 1 for right


func _generate_name() -> String:
	return "GOAPFlank  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_flank_position = Vector2.ZERO
	_flank_direction = 0
	print("GOAP ACTION: Flank started")


func _tick(_delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		print("GOAP ACTION: Flank - target invalid!")
		return FAILURE

	# Calculate flank position on first tick or if target moved significantly
	if _flank_position == Vector2.ZERO or _flank_direction == 0:
		_calculate_flank_position(target_node)

	var distance: float = agent.global_position.distance_to(_flank_position)

	if distance < tolerance:
		agent.velocity = Vector2.ZERO
		print("GOAP ACTION: Flank - reached flank position, target should be visible")
		return SUCCESS

	var direction: Vector2 = agent.global_position.direction_to(_flank_position)
	agent.velocity = direction * speed

	# Update facing toward target
	var face_dir: Vector2 = agent.global_position.direction_to(target_node.global_position)
	if face_dir.x > 0.1 and agent.root.scale.x < 0:
		agent.root.scale.x = 1.0
	elif face_dir.x < -0.1 and agent.root.scale.x > 0:
		agent.root.scale.x = -1.0

	agent.animation_player.play(&"walk")
	return RUNNING


func _calculate_flank_position(target_node: Node2D) -> void:
	var to_target: Vector2 = target_node.global_position - agent.global_position

	# Get perpendicular direction (both options)
	var perp_right: Vector2 = Vector2(-to_target.y, to_target.x).normalized()
	var perp_left: Vector2 = Vector2(to_target.y, -to_target.x).normalized()

	# Calculate both possible flank positions
	var mid_point: Vector2 = agent.global_position + to_target * 0.5
	var flank_right: Vector2 = mid_point + perp_right * flank_distance
	var flank_left: Vector2 = mid_point + perp_left * flank_distance

	# Choose the flank direction that keeps us more in bounds and closer to current position
	# Prefer the side we're already slightly on
	var current_side: float = signf(perp_right.dot(agent.global_position - mid_point))

	if current_side >= 0:
		_flank_position = flank_right
		_flank_direction = 1
	else:
		_flank_position = flank_left
		_flank_direction = -1

	# Clamp to arena bounds (approximate)
	_flank_position.x = clampf(_flank_position.x, 100.0, 1300.0)
	_flank_position.y = clampf(_flank_position.y, 150.0, 600.0)

	print("GOAP ACTION: Flank - calculated flank position at %s (direction: %d)" % [_flank_position, _flank_direction])
