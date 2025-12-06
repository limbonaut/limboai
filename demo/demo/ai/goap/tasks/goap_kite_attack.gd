@tool
extends BTAction
## Kite Attack: Shoots while backing away from melee threats.
## Combines retreat movement with ranged shooting for optimal kiting behavior.
## Returns RUNNING while kiting, SUCCESS when safe distance reached or target dead.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const ArenaUtilityClass = preload("res://demo/ai/goap/arena_utility.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Damage dealt by attack
@export var damage := 35

## Cooldown between shots in seconds
@export var shot_cooldown := 0.3

## Ninja star scene to spawn
const NINJA_STAR_SCENE = preload("res://demo/agents/ninja_star/ninja_star.tscn")

var _cooldown_timer := 0.0


func _generate_name() -> String:
	return "GOAPKiteAttack  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_cooldown_timer = 0.0
	print("GOAP: Starting kite attack!")


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	# Check if we have ranged weapon and ammo
	if not agent.has_node("CombatComponent"):
		return FAILURE
	var combat = agent.get_node("CombatComponent")
	if not combat.is_ranged():
		print("GOAP: Kite attack requires ranged weapon!")
		return FAILURE

	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()
	var distance: float = agent.global_position.distance_to(target_node.global_position)

	# Check if we've reached safe distance
	if distance >= GOAPConfigClass.RETREAT_DISTANCE:
		agent.velocity = Vector2.ZERO
		print("GOAP: Kite attack - safe distance reached!")
		return SUCCESS

	# Get movement speed (ranged is faster)
	var speed: float = combat.get_move_speed()

	# Calculate smart retreat direction using edge awareness
	var retreat_dir := _calculate_smart_kite_direction(dir_to_target, delta, speed)

	# Apply retreat movement
	if retreat_dir.length() > 0.1:
		agent.velocity = retreat_dir * speed
	else:
		agent.velocity = Vector2.ZERO

	# Keep facing the target while retreating
	_update_facing(dir_to_target)

	# Play walk animation
	if "animation_player" in agent:
		agent.animation_player.play(&"walk")

	# Handle shooting while retreating
	_cooldown_timer -= delta
	if _cooldown_timer <= 0.0:
		if _shoot_at_target(target_node, dir_to_target):
			_cooldown_timer = shot_cooldown
		else:
			# Out of ammo or jammed - return success to let GOAP replan
			print("GOAP: Kite attack - out of ammo!")
			return SUCCESS

	return RUNNING


func _shoot_at_target(target_node: Node2D, dir_to_target: Vector2) -> bool:
	# Consume ammo
	if agent.has_method("use_ammo"):
		if not agent.use_ammo():
			return false

	# Spawn ninja star
	var star = NINJA_STAR_SCENE.instantiate()
	var spawn_offset := dir_to_target * 60.0
	star.global_position = agent.global_position + spawn_offset + Vector2(0, -40)
	star.direction = dir_to_target
	star.shooter = agent
	agent.get_parent().add_child(star)

	# Apply suppression to target
	if target_node.has_node("CombatComponent"):
		var target_combat = target_node.get_node("CombatComponent")
		if target_combat.has_method("apply_suppression"):
			target_combat.apply_suppression()

	print("GOAP: Kite attack - threw ninja star!")
	return true


## Updates agent facing direction
func _update_facing(dir_to_target: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir_to_target.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0


## Calculates smart kite direction considering arena edges
## When near an edge, prefers strafing along the wall toward center rather than backing into corner
func _calculate_smart_kite_direction(dir_to_target: Vector2, _delta: float, _speed: float) -> Vector2:
	var pos: Vector2 = agent.global_position

	# Check if we're near arena edges using ArenaUtility
	var near_edge: bool = blackboard.get_var(&"near_edge", false)
	var cornered: bool = blackboard.get_var(&"cornered", false)
	var escape_direction: Vector2 = blackboard.get_var(&"escape_direction", Vector2.ZERO)

	# If cornered or near edge, use the pre-calculated escape direction
	if cornered and escape_direction != Vector2.ZERO:
		return escape_direction

	# Default retreat direction (away from target)
	var retreat_dir := -dir_to_target

	# If near edge but not cornered, blend retreat with strafe toward center
	if near_edge:
		# Get direction to arena center
		var arena_center: Vector2 = ArenaUtilityClass.get_arena_center()
		var dir_to_center: Vector2 = (arena_center - pos).normalized()

		# Calculate perpendicular strafe directions
		var strafe_left := Vector2(-dir_to_target.y, dir_to_target.x)
		var strafe_right := Vector2(dir_to_target.y, -dir_to_target.x)

		# Choose strafe direction that moves toward center
		var best_strafe: Vector2
		if strafe_left.dot(dir_to_center) > strafe_right.dot(dir_to_center):
			best_strafe = strafe_left
		else:
			best_strafe = strafe_right

		# Blend retreat with strafe (more strafe when near edge)
		var edge_distance: float = blackboard.get_var(&"min_edge_distance", 200.0)
		var edge_factor := 1.0 - clampf(edge_distance / GOAPConfigClass.EDGE_WARNING_DISTANCE, 0.0, 1.0)

		# edge_factor: 0 = far from edge (pure retreat), 1 = at edge (mostly strafe)
		retreat_dir = (retreat_dir * (1.0 - edge_factor * 0.7) + best_strafe * edge_factor * 0.7).normalized()

	# Final safety clamp to arena bounds
	var next_pos: Vector2 = pos + retreat_dir * 50.0  # Test 50 units ahead
	if not ArenaUtilityClass.is_position_in_bounds(next_pos, 20.0):
		# Use ArenaUtility's smart escape which handles corners and walls
		retreat_dir = ArenaUtilityClass.calculate_escape_direction(pos, dir_to_target)

	return retreat_dir
