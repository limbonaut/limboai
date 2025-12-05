@tool
extends BTAction
## Moves to the NEAREST available weapon pickup.
## Agents grab whatever weapon is closest, then adapt tactics after pickup.
## Stores the chosen pickup in weapon_pickup blackboard variable for the pickup task.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")

## Tolerance for reaching the target
@export var tolerance := 80.0

## Movement speed
@export var speed := 300.0


func _generate_name() -> String:
	return "GOAPGoToNearestWeapon"


func _enter() -> void:
	# Get both weapon pickups from blackboard
	var melee_pickup: Node2D = blackboard.get_var(&"melee_weapon_pickup")
	var ranged_pickup: Node2D = blackboard.get_var(&"ranged_weapon_pickup")

	var target_pickup: Node2D = null
	var melee_available := is_instance_valid(melee_pickup) and _is_pickup_available(melee_pickup)
	var ranged_available := is_instance_valid(ranged_pickup) and _is_pickup_available(ranged_pickup)

	# Pick the NEAREST available weapon
	if melee_available and ranged_available:
		var dist_to_melee: float = agent.global_position.distance_to(melee_pickup.global_position)
		var dist_to_ranged: float = agent.global_position.distance_to(ranged_pickup.global_position)
		if dist_to_melee <= dist_to_ranged:
			target_pickup = melee_pickup
		else:
			target_pickup = ranged_pickup
	elif melee_available:
		target_pickup = melee_pickup
	elif ranged_available:
		target_pickup = ranged_pickup

	# Store in weapon_pickup for the pickup task to use
	blackboard.set_var(&"weapon_pickup", target_pickup)

	if target_pickup:
		var weapon_type := "MELEE" if target_pickup == melee_pickup else "RANGED"
		print("GOAP ACTION: GoTo %s (nearest: %s)" % [target_pickup.name, weapon_type])


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(&"weapon_pickup")
	if not is_instance_valid(target_node):
		print("GOAP: No weapon pickup available!")
		return FAILURE

	if not _is_pickup_available(target_node):
		print("GOAP: Weapon pickup not available!")
		return FAILURE

	var target_pos: Vector2 = target_node.global_position
	var distance: float = agent.global_position.distance_to(target_pos)

	if distance <= tolerance:
		# Reached destination
		agent.velocity = Vector2.ZERO
		return SUCCESS

	# Get speed from combat component (uses default if no weapon yet)
	var move_speed := speed
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			move_speed = combat.get_move_speed()

	# Move towards target
	var direction: Vector2 = (target_pos - agent.global_position).normalized()
	agent.velocity = direction * move_speed

	# Update facing
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if direction.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0

	# Play walk animation
	if "animation_player" in agent:
		agent.animation_player.play(&"walk")

	return RUNNING


func _is_pickup_available(pickup: Node2D) -> bool:
	if pickup.has_method("is_available"):
		return pickup.is_available()
	return true
