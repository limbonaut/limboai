## RetreatRanged action with dynamic cost based on threat type
## Retreat is the CORRECT response to melee threats for ranged agents
## Makes retreat very cheap when facing melee, so planner prefers it over cover
extends GOAPAction


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var melee_threat = blackboard.get_var(&"melee_threat", false)
	var enemy_has_melee = blackboard.get_var(&"enemy_has_melee_weapon", false)

	# Retreat is the RIGHT choice against melee - make it very cheap
	# This ensures the planner picks retreat over cover when facing melee
	if melee_threat or enemy_has_melee:
		return 1  # Minimum cost - retreat from melee!

	# Otherwise use base cost
	return p_base_cost
