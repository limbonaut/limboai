@tool
extends BTAction
## Loads ammo from an ammo pickup.
## Returns SUCCESS after loading ammo.

## Blackboard variable storing the ammo pickup node
@export var pickup_var := &"ammo_pickup"

## Amount of ammo to add
@export var ammo_amount := 10


func _generate_name() -> String:
	return "GOAPLoadAmmo  pickup: %s  amount: %d" % [
		LimboUtility.decorate_var(pickup_var),
		ammo_amount
	]


func _tick(_delta: float) -> Status:
	var pickup_node: Node2D = blackboard.get_var(pickup_var)

	# Add ammo to the agent
	if agent.has_method("add_ammo"):
		agent.add_ammo(ammo_amount)

	# Collect the pickup (hides and starts respawn timer)
	if is_instance_valid(pickup_node):
		if pickup_node.has_method("collect"):
			pickup_node.collect()
		else:
			# Fallback for old pickups without collect method
			pickup_node.queue_free()
			blackboard.set_var(pickup_var, null)

	print("GOAP: Loaded %d ammo" % ammo_amount)
	return SUCCESS
