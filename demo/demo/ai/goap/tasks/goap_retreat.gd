@tool
extends BTAction
## Tactical Retreat: Circles around melee threats while heading toward resources.
## Instead of just backing up, strafes perpendicular to maintain distance while
## making progress toward ammo or other objectives.
##
## Behavior:
## - Very close (danger zone): Pure retreat to create space
## - Medium range: Circle strafe toward nearest resource (ammo preferred)
## - Safe range: Success - can transition to next action

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const ArenaUtilityClass = preload("res://demo/ai/goap/arena_utility.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Blackboard variable storing ammo pickup (preferred destination while evading)
@export var ammo_var := &"ammo_pickup"

## Fallback retreat speed (uses combat component speed when available)
@export var retreat_speed := 300.0

## Distance at which we switch to pure retreat (too close!)
@export var danger_distance := 150.0

## Safe distance - we've evaded successfully
@export var safe_distance := 350.0

## How much to weight circling vs retreating (0 = pure retreat, 1 = pure strafe)
@export var circle_weight := 0.6

# Escape state for corner handling
var _escape_direction: Vector2 = Vector2.ZERO
var _escape_timer: float = 0.0


func _generate_name() -> String:
	return "GOAPRetreat  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_escape_direction = Vector2.ZERO
	_escape_timer = 0.0


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	var to_target: Vector2 = target_node.global_position - agent.global_position
	var distance: float = to_target.length()
	var dir_to_target: Vector2 = to_target.normalized()

	# Check if we've retreated far enough
	if distance >= safe_distance:
		agent.velocity = Vector2.ZERO
		return SUCCESS

	# Get speed from combat component (ranged is faster for kiting)
	var speed := retreat_speed
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			speed = combat.get_move_speed()

	# Decrement escape timer
	if _escape_timer > 0.0:
		_escape_timer -= delta
	else:
		_escape_direction = Vector2.ZERO

	# Calculate movement direction with smart corner/wall handling
	var move_dir: Vector2 = _calculate_smart_retreat(dir_to_target, distance, delta)

	# If we can't move anywhere useful, just stop
	if move_dir.length() < 0.1:
		agent.velocity = Vector2.ZERO
		return SUCCESS

	agent.velocity = move_dir * speed

	# Keep facing the target while retreating/circling
	_update_facing(dir_to_target)

	# Play walk animation
	if agent.has_node("AnimationPlayer"):
		var anim: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim.has_animation(&"walk"):
			anim.play(&"walk")

	return RUNNING


## Calculates the best direction to circle around the threat
## Chooses the strafe direction that moves us toward resources (ammo preferred)
func _calculate_circle_direction(dir_to_target: Vector2, distance: float) -> Vector2:
	# Calculate perpendicular directions (strafe left and right)
	var strafe_left: Vector2 = Vector2(-dir_to_target.y, dir_to_target.x)
	var strafe_right: Vector2 = Vector2(dir_to_target.y, -dir_to_target.x)

	# Retreat component (away from target)
	var retreat_dir: Vector2 = -dir_to_target

	# Find the best strafe direction based on resources
	var best_strafe: Vector2 = _choose_strafe_toward_resource(strafe_left, strafe_right)

	# Blend retreat and strafe based on distance
	# Closer = more retreat, farther = more strafe
	var distance_factor: float = clampf((distance - danger_distance) / (safe_distance - danger_distance), 0.0, 1.0)
	var actual_circle_weight: float = circle_weight * distance_factor

	# Combine retreat and strafe
	var combined: Vector2 = retreat_dir * (1.0 - actual_circle_weight) + best_strafe * actual_circle_weight
	return combined.normalized()


## Chooses which strafe direction moves us closer to resources
func _choose_strafe_toward_resource(strafe_left: Vector2, strafe_right: Vector2) -> Vector2:
	var best_strafe: Vector2 = strafe_left  # Default
	var best_score: float = -INF

	# Check ammo pickup first (highest priority when evading)
	var ammo_pickup: Node2D = blackboard.get_var(ammo_var, null)
	if is_instance_valid(ammo_pickup):
		if not ammo_pickup.has_method("is_available") or ammo_pickup.is_available():
			var dir_to_ammo: Vector2 = (ammo_pickup.global_position - agent.global_position).normalized()
			var left_score: float = strafe_left.dot(dir_to_ammo)
			var right_score: float = strafe_right.dot(dir_to_ammo)

			if left_score > right_score:
				return strafe_left
			else:
				return strafe_right

	# Check health pickup as secondary
	var health_pickup: Node2D = blackboard.get_var(&"health_pickup", null)
	if is_instance_valid(health_pickup):
		if not health_pickup.has_method("is_available") or health_pickup.is_available():
			var dir_to_health: Vector2 = (health_pickup.global_position - agent.global_position).normalized()
			var left_score: float = strafe_left.dot(dir_to_health)
			var right_score: float = strafe_right.dot(dir_to_health)

			if left_score > right_score:
				return strafe_left
			else:
				return strafe_right

	# No resources - pick based on arena center (stay in middle)
	var arena_center: Vector2 = (GOAPConfigClass.ARENA_MIN + GOAPConfigClass.ARENA_MAX) / 2.0
	var dir_to_center: Vector2 = (arena_center - agent.global_position).normalized()
	var left_score: float = strafe_left.dot(dir_to_center)
	var right_score: float = strafe_right.dot(dir_to_center)

	if left_score > right_score:
		return strafe_left
	else:
		return strafe_right


## Calculates retreat direction with smart corner/wall handling
## Uses ArenaUtility to detect corners and calculate escape routes
func _calculate_smart_retreat(dir_to_target: Vector2, distance: float, _delta: float) -> Vector2:
	var position: Vector2 = agent.global_position

	# Check if we're in a corner - use cached escape direction or calculate new one
	if ArenaUtilityClass.is_in_corner(position):
		if _escape_direction == Vector2.ZERO or _escape_timer <= 0.0:
			_escape_direction = ArenaUtilityClass.calculate_escape_direction(position, dir_to_target)
			_escape_timer = ArenaUtilityClass.ESCAPE_DURATION
			print("GOAP Retreat: Corner detected! Escaping %s" % _escape_direction)
		return _escape_direction

	# Check if we're near a wall (but not cornered)
	var wall_flags := ArenaUtilityClass.get_wall_flags(position)
	if wall_flags != ArenaUtilityClass.WallFlags.NONE:
		# Use ArenaUtility's smart escape which slides along walls
		return ArenaUtilityClass.calculate_escape_direction(position, dir_to_target)

	# Not near walls - use standard retreat/circle logic
	if distance < danger_distance:
		# DANGER ZONE: Pure retreat - just get away!
		return -dir_to_target
	else:
		# SAFE ENOUGH TO CIRCLE: Strafe around threat toward resources
		return _calculate_circle_direction(dir_to_target, distance)


## Updates agent facing direction to always face the threat
func _update_facing(dir_to_target: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir_to_target.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0
