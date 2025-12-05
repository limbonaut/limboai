## LoadWeapon action with dynamic cost based on ACTUAL weapon type
## Melee weapons don't need loading - make this action very expensive for melee agents
## Only checks equipped weapon, not preferred mode (agents grab nearest weapon)
extends GOAPAction


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	# Check if agent has melee weapon - melee doesn't need to load!
	var has_melee = blackboard.get_var(&"has_melee_weapon", false)
	if has_melee:
		return 1000  # Effectively disable this action for melee agents

	# Ranged agents (or agents without weapon yet) use base cost
	return p_base_cost
