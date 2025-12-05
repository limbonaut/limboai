@tool
extends BTAction
## Moves to the appropriate weapon pickup based on agent's preferred combat mode.
## Checks CombatComponent.preferred_mode and goes to melee or ranged pickup.
## Stores the chosen pickup in weapon_pickup blackboard variable for the pickup task.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")

## Tolerance for reaching the target
@export var tolerance := 80.0

## Movement speed
@export var speed := 300.0


func _generate_name() -> String:
	return "GOAPGoToPreferredWeapon"


func _enter() -> void:
	# Determine which weapon to go to based on preferred mode
	var combat: CombatComponentClass = null
	if agent.has_node("CombatComponent"):
		combat = agent.get_node("CombatComponent")

	var preferred_mode := CombatComponentClass.CombatMode.RANGED
	if combat:
		preferred_mode = combat.preferred_mode

	# Get the appropriate weapon pickup from blackboard
	var melee_pickup: Node2D = blackboard.get_var(&"melee_weapon_pickup")
	var ranged_pickup: Node2D = blackboard.get_var(&"ranged_weapon_pickup")

	var target_pickup: Node2D = null

	# Check if preferred weapon is available, otherwise use whatever is available
	if preferred_mode == CombatComponentClass.CombatMode.MELEE:
		if is_instance_valid(melee_pickup) and _is_pickup_available(melee_pickup):
			target_pickup = melee_pickup
		elif is_instance_valid(ranged_pickup) and _is_pickup_available(ranged_pickup):
			target_pickup = ranged_pickup  # Fallback to ranged
	else:  # RANGED
		if is_instance_valid(ranged_pickup) and _is_pickup_available(ranged_pickup):
			target_pickup = ranged_pickup
		elif is_instance_valid(melee_pickup) and _is_pickup_available(melee_pickup):
			target_pickup = melee_pickup  # Fallback to melee

	# Store in weapon_pickup for the pickup task to use
	blackboard.set_var(&"weapon_pickup", target_pickup)

	if target_pickup:
		print("GOAP ACTION: GoTo %s (preferred: %s)" % [target_pickup.name, "MELEE" if preferred_mode == CombatComponentClass.CombatMode.MELEE else "RANGED"])


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

	# Move towards target
	var direction: Vector2 = (target_pos - agent.global_position).normalized()
	agent.velocity = direction * speed

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
