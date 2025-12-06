@tool
extends BTAction
## Evade To Ammo: Moves toward ammo pickup while evading melee threats.
## Blends evasion and goal-directed movement for ranged agents without ammo.
## If melee gets too close, prioritizes evasion over ammo path.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const ArenaUtilityClass = preload("res://demo/ai/goap/arena_utility.gd")

## Blackboard variable storing the target (enemy)
@export var target_var := &"target"

## Blackboard variable storing the ammo pickup
@export var ammo_pickup_var := &"ammo_pickup"

## Tolerance for reaching ammo
@export var tolerance := 80.0

## Distance at which we prioritize evasion over ammo path
@export var danger_distance := 200.0


func _generate_name() -> String:
	return "GOAPEvadeToAmmo"


func _enter() -> void:
	print("GOAP: Evading to ammo!")


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	var ammo_pickup: Node2D = blackboard.get_var(ammo_pickup_var)

	# Need ammo pickup to be valid
	if not is_instance_valid(ammo_pickup):
		print("GOAP: EvadeToAmmo - no ammo pickup!")
		return FAILURE

	# Check if ammo is available
	if ammo_pickup.has_method("is_available") and not ammo_pickup.is_available():
		print("GOAP: EvadeToAmmo - ammo not available!")
		return FAILURE

	var ammo_pos: Vector2 = ammo_pickup.global_position
	var distance_to_ammo: float = agent.global_position.distance_to(ammo_pos)

	# Check if we've reached the ammo
	if distance_to_ammo <= tolerance:
		agent.velocity = Vector2.ZERO
		# Actually pick up the ammo
		if agent.has_method("add_ammo"):
			agent.add_ammo(10)
		if ammo_pickup.has_method("collect"):
			ammo_pickup.collect()
		print("GOAP: EvadeToAmmo - collected ammo!")
		return SUCCESS

	# Get movement speed (ranged is faster)
	var speed: float = GOAPConfigClass.RANGED_MOVE_SPEED
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			speed = combat.get_move_speed()

	# Calculate base direction to ammo
	var dir_to_ammo: Vector2 = (ammo_pos - agent.global_position).normalized()
	var move_dir: Vector2 = dir_to_ammo

	# If there's a threat, use smart pathing that considers ammo direction
	if is_instance_valid(target_node):
		var dir_to_threat: Vector2 = (target_node.global_position - agent.global_position).normalized()
		var distance_to_threat: float = agent.global_position.distance_to(target_node.global_position)
		var away_from_threat: Vector2 = -dir_to_threat

		# Check if ammo is in a "safe" direction (not toward the threat)
		# Dot product: 1.0 = same direction, -1.0 = opposite direction
		var ammo_safety: float = dir_to_ammo.dot(away_from_threat)
		# Convert from [-1, 1] to [0, 1] - higher = safer direction
		var safety_factor: float = (ammo_safety + 1.0) / 2.0

		# Weight ammo more heavily when:
		# 1. Ammo is in a safe direction (away from threat)
		# 2. We're close to the ammo
		# 3. We have speed advantage (ranged is faster)
		var proximity_factor: float = clampf(1.0 - (distance_to_ammo / 400.0), 0.2, 1.0)
		var ammo_weight: float = clampf(safety_factor * 0.6 + proximity_factor * 0.4, 0.3, 0.8)

		if distance_to_threat < danger_distance:
			# Very close - still evade but maintain some ammo progress
			move_dir = (away_from_threat * (1.0 - ammo_weight * 0.5) + dir_to_ammo * ammo_weight * 0.5).normalized()
		else:
			# Safer distance - prioritize ammo more
			move_dir = (away_from_threat * (1.0 - ammo_weight) + dir_to_ammo * ammo_weight).normalized()

		# Update facing to look at threat
		_update_facing(dir_to_threat)
	else:
		# No threat - face movement direction
		_update_facing(dir_to_ammo)

	# Check arena bounds and use smart edge handling
	var pos: Vector2 = agent.global_position
	var next_pos: Vector2 = pos + move_dir * speed * delta
	if not ArenaUtilityClass.is_position_in_bounds(next_pos, 20.0):
		# Use smart escape direction that considers walls and threat
		var threat_dir := Vector2.ZERO
		if is_instance_valid(target_node):
			threat_dir = (target_node.global_position - pos).normalized()
		move_dir = ArenaUtilityClass.calculate_escape_direction(pos, threat_dir)

		# Still try to bias toward ammo if possible
		var adjusted_dir: Vector2 = (move_dir * 0.6 + dir_to_ammo * 0.4).normalized()
		var test_pos: Vector2 = pos + adjusted_dir * speed * delta
		if ArenaUtilityClass.is_position_in_bounds(test_pos, 20.0):
			move_dir = adjusted_dir

	# Apply movement
	agent.velocity = move_dir * speed

	# Play walk animation
	if "animation_player" in agent:
		agent.animation_player.play(&"walk")

	return RUNNING


## Updates agent facing direction
func _update_facing(dir: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0
