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


func _generate_name() -> String:
	return "GOAPRetreat  target: %s" % LimboUtility.decorate_var(target_var)


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

	# Calculate movement direction based on threat distance
	var move_dir: Vector2

	if distance < danger_distance:
		# DANGER ZONE: Pure retreat - just get away!
		move_dir = -dir_to_target
	else:
		# SAFE ENOUGH TO CIRCLE: Strafe around threat toward resources
		move_dir = _calculate_circle_direction(dir_to_target, distance)

	# Apply arena bounds
	move_dir = _apply_bounds(move_dir, speed * delta)

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


## Applies arena boundary constraints to movement direction
func _apply_bounds(move_dir: Vector2, step_size: float) -> Vector2:
	var next_pos: Vector2 = agent.global_position + move_dir * step_size
	var adjusted: Vector2 = move_dir

	# Bounce off horizontal bounds
	if next_pos.x < GOAPConfigClass.ARENA_MIN.x:
		adjusted.x = absf(adjusted.x)  # Force positive (move right)
	elif next_pos.x > GOAPConfigClass.ARENA_MAX.x:
		adjusted.x = -absf(adjusted.x)  # Force negative (move left)

	# Bounce off vertical bounds
	if next_pos.y < GOAPConfigClass.ARENA_MIN.y:
		adjusted.y = absf(adjusted.y)  # Force positive (move down)
	elif next_pos.y > GOAPConfigClass.ARENA_MAX.y:
		adjusted.y = -absf(adjusted.y)  # Force negative (move up)

	return adjusted.normalized() if adjusted.length() > 0.1 else Vector2.ZERO


## Updates agent facing direction to always face the threat
func _update_facing(dir_to_target: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir_to_target.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0
