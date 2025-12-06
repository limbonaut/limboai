## Arena Utility
## Provides spatial reasoning functions for arena boundary awareness.
## Used by movement tasks to detect walls, corners, and calculate escape routes.
class_name ArenaUtility
extends RefCounted

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

## Wall detection threshold - how close to a single wall to be considered "near wall"
const WALL_THRESHOLD := 80.0

## Corner detection threshold - how close to two walls to be considered "in corner"
const CORNER_THRESHOLD := 120.0

## Duration to maintain escape direction to prevent oscillation
const ESCAPE_DURATION := 0.5

## Wall flags bitfield
enum WallFlags {
	NONE = 0,
	LEFT = 1,
	RIGHT = 2,
	TOP = 4,
	BOTTOM = 8
}


## Returns bitfield of which walls are near the given position
static func get_wall_flags(position: Vector2, threshold: float = WALL_THRESHOLD) -> int:
	var flags := WallFlags.NONE

	if position.x < GOAPConfigClass.ARENA_MIN.x + threshold:
		flags |= WallFlags.LEFT
	if position.x > GOAPConfigClass.ARENA_MAX.x - threshold:
		flags |= WallFlags.RIGHT
	if position.y < GOAPConfigClass.ARENA_MIN.y + threshold:
		flags |= WallFlags.TOP
	if position.y > GOAPConfigClass.ARENA_MAX.y - threshold:
		flags |= WallFlags.BOTTOM

	return flags


## Returns true if position is near two perpendicular walls (in a corner)
static func is_in_corner(position: Vector2, threshold: float = CORNER_THRESHOLD) -> bool:
	var flags := get_wall_flags(position, threshold)

	# Check for corner combinations (two perpendicular walls)
	var near_horizontal := (flags & WallFlags.LEFT) or (flags & WallFlags.RIGHT)
	var near_vertical := (flags & WallFlags.TOP) or (flags & WallFlags.BOTTOM)

	return near_horizontal and near_vertical


## Returns corner type as string: "top_left", "top_right", "bottom_left", "bottom_right", or ""
static func get_corner_type(position: Vector2, threshold: float = CORNER_THRESHOLD) -> String:
	var flags := get_wall_flags(position, threshold)

	if (flags & WallFlags.LEFT) and (flags & WallFlags.TOP):
		return "top_left"
	elif (flags & WallFlags.RIGHT) and (flags & WallFlags.TOP):
		return "top_right"
	elif (flags & WallFlags.LEFT) and (flags & WallFlags.BOTTOM):
		return "bottom_left"
	elif (flags & WallFlags.RIGHT) and (flags & WallFlags.BOTTOM):
		return "bottom_right"

	return ""


## Returns the center of the arena
static func get_arena_center() -> Vector2:
	return (GOAPConfigClass.ARENA_MIN + GOAPConfigClass.ARENA_MAX) / 2.0


## Clamps a position to arena bounds
static func clamp_to_arena(position: Vector2) -> Vector2:
	return Vector2(
		clampf(position.x, GOAPConfigClass.ARENA_MIN.x, GOAPConfigClass.ARENA_MAX.x),
		clampf(position.y, GOAPConfigClass.ARENA_MIN.y, GOAPConfigClass.ARENA_MAX.y)
	)


## Returns true if position is within arena bounds (with optional margin)
static func is_position_in_bounds(position: Vector2, margin: float = 0.0) -> bool:
	return (position.x >= GOAPConfigClass.ARENA_MIN.x + margin and
			position.x <= GOAPConfigClass.ARENA_MAX.x - margin and
			position.y >= GOAPConfigClass.ARENA_MIN.y + margin and
			position.y <= GOAPConfigClass.ARENA_MAX.y - margin)


## Calculates a smart escape direction when cornered or near walls
## Returns a direction vector that moves along walls toward the arena center
## while avoiding the threat direction
static func calculate_escape_direction(position: Vector2, threat_dir: Vector2) -> Vector2:
	var flags := get_wall_flags(position, CORNER_THRESHOLD)
	var center := get_arena_center()
	var to_center := (center - position).normalized()

	if is_in_corner(position, CORNER_THRESHOLD):
		# In corner: calculate escape options perpendicular to each wall
		var escape_options: Array[Vector2] = []

		# Add directions that move AWAY from each nearby wall
		if flags & WallFlags.LEFT:
			escape_options.append(Vector2.RIGHT)
		if flags & WallFlags.RIGHT:
			escape_options.append(Vector2.LEFT)
		if flags & WallFlags.TOP:
			escape_options.append(Vector2.DOWN)
		if flags & WallFlags.BOTTOM:
			escape_options.append(Vector2.UP)

		# Score each option: prefer directions away from threat AND toward center
		var best_dir := Vector2.ZERO
		var best_score := -INF

		for dir in escape_options:
			var away_score := dir.dot(-threat_dir)  # Away from threat
			var center_score := dir.dot(to_center)   # Toward center
			var score := away_score * 0.6 + center_score * 0.4
			if score > best_score:
				best_score = score
				best_dir = dir

		return best_dir

	elif flags != WallFlags.NONE:
		# Near single wall but not cornered: slide along wall while retreating
		var retreat := -threat_dir

		# Project retreat direction to slide along wall
		if flags & WallFlags.LEFT:
			# Near left wall - only allow positive X (moving right) or zero
			retreat.x = maxf(retreat.x, 0.0)
		if flags & WallFlags.RIGHT:
			# Near right wall - only allow negative X (moving left) or zero
			retreat.x = minf(retreat.x, 0.0)
		if flags & WallFlags.TOP:
			# Near top wall - only allow positive Y (moving down) or zero
			retreat.y = maxf(retreat.y, 0.0)
		if flags & WallFlags.BOTTOM:
			# Near bottom wall - only allow negative Y (moving up) or zero
			retreat.y = minf(retreat.y, 0.0)

		# If we can't retreat at all, move toward center
		if retreat.length() < 0.1:
			retreat = to_center

		# Add center bias to help escape walls
		return (retreat.normalized() * 0.7 + to_center * 0.3).normalized()

	# Not near walls - return simple retreat direction
	return -threat_dir


## Finds a safe position away from a threat, staying within arena bounds
## Returns a target position that is at least min_dist from away_from
static func find_safe_position(from_pos: Vector2, away_from: Vector2, min_dist: float) -> Vector2:
	var escape_dir := calculate_escape_direction(from_pos, (away_from - from_pos).normalized())
	var target_pos := from_pos + escape_dir * min_dist
	return clamp_to_arena(target_pos)
