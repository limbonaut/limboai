@tool
extends BTAction
## Performs a melee attack on the target.
## Requires being in melee range. No ammo cost.

## Blackboard variable storing the target node
@export var target_var := &"target"

## Damage dealt by melee attack
@export var damage := 35


func _generate_name() -> String:
	return "GOAPMelee  target: %s" % LimboUtility.decorate_var(target_var)


func _tick(_delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	# Update agent facing toward target
	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if dir_to_target.x > 0:
			root.scale.x = scale_magnitude
		else:
			root.scale.x = -scale_magnitude

	# Play melee swing animation
	if agent.has_method("play_melee_swing"):
		agent.play_melee_swing()

	# Deal damage to target
	if target_node.has_node("Health"):
		var health = target_node.get_node("Health")
		if health.has_method("take_damage"):
			health.take_damage(damage, Vector2.ZERO)
			print("GOAP: Melee attack dealt %d damage!" % damage)
			return SUCCESS

	return FAILURE
