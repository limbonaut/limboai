@tool
extends BTAction
## Picks up a health item and heals the agent.

## Blackboard variable storing the health pickup node
@export var pickup_var := &"health_pickup"

## Amount to heal
@export var heal_amount := 50


func _generate_name() -> String:
	return "GOAPHeal  pickup: %s" % LimboUtility.decorate_var(pickup_var)


func _tick(_delta: float) -> Status:
	var pickup_node: Node2D = blackboard.get_var(pickup_var)
	if not is_instance_valid(pickup_node):
		return FAILURE

	# Heal the agent
	if agent.has_method("heal"):
		agent.heal(heal_amount)

	# Collect the pickup (hides and starts respawn timer)
	if pickup_node.has_method("collect"):
		pickup_node.collect()
	else:
		# Fallback for old pickups without collect method
		pickup_node.queue_free()

	print("GOAP: Picked up health, healed %d!" % heal_amount)
	return SUCCESS
