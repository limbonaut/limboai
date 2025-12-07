@tool
extends BTAction
## Moves the GOAP agent to a position with clear line of sight to the target.
## Used when LoS is blocked by cover objects.
## Returns SUCCESS when LoS achieved, RUNNING while repositioning.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

## Blackboard variable that stores the target node (Node2D)
@export var target_var := &"target"

## How close should the agent be to the target position to consider it reached
@export var tolerance := 30.0

## Movement speed
@export var speed := 280.0

## How far to search for a clear position
@export var search_distance := 150.0

var _target_position: Vector2 = Vector2.ZERO
var _search_attempts := 0
const MAX_SEARCH_ATTEMPTS := 8


func _generate_name() -> String:
	return "GOAPGetLOS  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_target_position = Vector2.ZERO
	_search_attempts = 0
	print("GOAP ACTION: GetLOS started - finding clear firing position")


func _tick(_delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		print("GOAP ACTION: GetLOS - target invalid!")
		return FAILURE

	# Check if we already have LoS
	if _has_los_to(target_node.global_position):
		agent.velocity = Vector2.ZERO
		print("GOAP ACTION: GetLOS - line of sight achieved!")
		return SUCCESS

	# Find a position with clear LoS if we don't have one yet
	if _target_position == Vector2.ZERO:
		_target_position = _find_los_position(target_node)
		if _target_position == Vector2.ZERO:
			# No clear position found, try moving perpendicular
			_target_position = _get_perpendicular_position(target_node)
			print("GOAP ACTION: GetLOS - no clear position, trying perpendicular move")

	# Move toward target position
	var distance: float = agent.global_position.distance_to(_target_position)

	if distance < tolerance:
		# Reached position but still no LoS - recalculate
		_search_attempts += 1
		if _search_attempts >= MAX_SEARCH_ATTEMPTS:
			print("GOAP ACTION: GetLOS - failed after %d attempts" % MAX_SEARCH_ATTEMPTS)
			return FAILURE
		_target_position = Vector2.ZERO
		return RUNNING

	var direction: Vector2 = agent.global_position.direction_to(_target_position)
	agent.velocity = direction * speed

	# Update facing toward target
	var face_dir: Vector2 = agent.global_position.direction_to(target_node.global_position)
	var scale_magnitude := absf(agent.root.scale.x)
	if face_dir.x > 0.1 and agent.root.scale.x < 0:
		agent.root.scale.x = scale_magnitude
	elif face_dir.x < -0.1 and agent.root.scale.x > 0:
		agent.root.scale.x = -scale_magnitude

	agent.animation_player.play(&"walk")
	return RUNNING


## Checks if there's a clear line of sight to a position (from projectile spawn point)
func _has_los_to(target_pos: Vector2) -> bool:
	var space_state = agent.get_world_2d().direct_space_state
	# Check from projectile spawn position (matches goap_attack.gd spawn offset)
	var dir_to_target = (target_pos - agent.global_position).normalized()
	var spawn_offset = dir_to_target * 60.0
	var from_pos = agent.global_position + spawn_offset + Vector2(0, -40)
	var query = PhysicsRayQueryParameters2D.create(
		from_pos,
		target_pos,
		GOAPConfigClass.LOS_COLLISION_LAYER
	)
	var result = space_state.intersect_ray(query)
	return result.is_empty()


## Finds a nearby position with clear LoS to the target
func _find_los_position(target_node: Node2D) -> Vector2:
	var target_pos: Vector2 = target_node.global_position
	var best_pos := Vector2.ZERO
	var best_dist := INF

	# Try positions in a circle around current location
	for i in range(8):
		var angle: float = i * TAU / 8.0
		var offset := Vector2(cos(angle), sin(angle)) * search_distance
		var test_pos: Vector2 = agent.global_position + offset

		# Clamp to arena bounds
		test_pos.x = clampf(test_pos.x, GOAPConfigClass.ARENA_MIN.x, GOAPConfigClass.ARENA_MAX.x)
		test_pos.y = clampf(test_pos.y, GOAPConfigClass.ARENA_MIN.y, GOAPConfigClass.ARENA_MAX.y)

		# Check if this position has LoS to target
		if _position_has_los(test_pos, target_pos):
			var dist: float = agent.global_position.distance_to(test_pos)
			if dist < best_dist:
				best_dist = dist
				best_pos = test_pos

	return best_pos


## Checks if a given position has LoS to the target (accounting for projectile spawn offset)
func _position_has_los(from_pos: Vector2, target_pos: Vector2) -> bool:
	var space_state = agent.get_world_2d().direct_space_state
	# Check from projectile spawn position at this location
	var dir_to_target = (target_pos - from_pos).normalized()
	var spawn_offset = dir_to_target * 60.0
	var spawn_pos = from_pos + spawn_offset + Vector2(0, -40)
	var query = PhysicsRayQueryParameters2D.create(
		spawn_pos,
		target_pos,
		GOAPConfigClass.LOS_COLLISION_LAYER
	)
	var result = space_state.intersect_ray(query)
	return result.is_empty()


## Gets a position perpendicular to the line to target (for when no clear LoS found)
func _get_perpendicular_position(target_node: Node2D) -> Vector2:
	var to_target: Vector2 = target_node.global_position - agent.global_position
	var perp: Vector2 = Vector2(-to_target.y, to_target.x).normalized()

	# Choose direction based on which side has more space
	var pos_right: Vector2 = agent.global_position + perp * search_distance
	var pos_left: Vector2 = agent.global_position - perp * search_distance

	# Prefer the side that's more in bounds
	pos_right.x = clampf(pos_right.x, GOAPConfigClass.ARENA_MIN.x, GOAPConfigClass.ARENA_MAX.x)
	pos_right.y = clampf(pos_right.y, GOAPConfigClass.ARENA_MIN.y, GOAPConfigClass.ARENA_MAX.y)
	pos_left.x = clampf(pos_left.x, GOAPConfigClass.ARENA_MIN.x, GOAPConfigClass.ARENA_MAX.x)
	pos_left.y = clampf(pos_left.y, GOAPConfigClass.ARENA_MIN.y, GOAPConfigClass.ARENA_MAX.y)

	var dist_right: float = agent.global_position.distance_to(pos_right)
	var dist_left: float = agent.global_position.distance_to(pos_left)

	# Return the position that moved further (more room to maneuver)
	if dist_right > dist_left:
		return pos_right
	else:
		return pos_left
