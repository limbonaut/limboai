@tool
extends BTAction
## Picks up an item and sets a blackboard variable.
## Returns SUCCESS after picking up.
## For weapon pickups, also sets weapon_type on the CombatComponent.

const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")

## Blackboard variable storing the pickup node
@export var pickup_var := &"weapon_pickup"

## Blackboard variable to set to true after pickup
@export var result_var := &"has_weapon"

## Animation to play
@export var animation := &"pickup"


func _generate_name() -> String:
	return "GOAPPickup  item: %s  sets: %s" % [
		LimboUtility.decorate_var(pickup_var),
		LimboUtility.decorate_var(result_var)
	]


func _tick(_delta: float) -> Status:
	var pickup_node: Node2D = blackboard.get_var(pickup_var)

	# For weapon pickups, set weapon type on combat component
	if is_instance_valid(pickup_node) and "weapon_type" in pickup_node:
		if agent.has_node("CombatComponent"):
			var combat: CombatComponentClass = agent.get_node("CombatComponent")
			match pickup_node.weapon_type:
				"melee":
					combat.weapon_type = CombatComponentClass.WeaponType.MELEE
					print("GOAP: Equipped MELEE weapon")
				"ranged":
					combat.weapon_type = CombatComponentClass.WeaponType.RANGED
					print("GOAP: Equipped RANGED weapon")
				_:
					combat.weapon_type = CombatComponentClass.WeaponType.RANGED
					print("GOAP: Equipped default RANGED weapon")
			# Update weapon visual on agent
			if agent.has_method("update_weapon_visual"):
				agent.update_weapon_visual()

	# Collect the pickup (hides it and starts respawn timer)
	if is_instance_valid(pickup_node):
		if pickup_node.has_method("collect"):
			pickup_node.collect()
		else:
			# Fallback for old pickups without collect method
			pickup_node.queue_free()
			blackboard.set_var(pickup_var, null)

	# Set result
	blackboard.set_var(result_var, true)

	# Also set on agent if it has the property
	if result_var in agent:
		agent.set(result_var, true)

	# Play animation if agent has one
	if agent.has_node("AnimationPlayer"):
		var anim_player: AnimationPlayer = agent.get_node("AnimationPlayer")
		if anim_player.has_animation(animation):
			anim_player.play(animation)

	print("GOAP: Picked up %s, set %s = true" % [pickup_var, result_var])
	return SUCCESS
