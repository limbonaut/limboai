## GoToAmmo action with dynamic cost based on ACTUAL weapon type
## Melee weapons don't use ammo - make this action very expensive for melee agents
## Only checks equipped weapon, not preferred mode (agents grab nearest weapon)
##
## Supports WeightGenome optimization:
## - cost_ammo_with_melee: Cost when melee agent tries to get ammo (very high = avoid)
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	# Check if agent has melee weapon - melee doesn't need ammo!
	var has_melee = blackboard.get_var(&"has_melee_weapon", false)
	if has_melee:
		# Use genome weight if available, otherwise use optimized config constant
		return WeightApplicatorClass.get_weight(agent, "cost_ammo_with_melee", GOAPConfigClass.COST_AMMO_WITH_MELEE)

	# Ranged agents (or agents without weapon yet) use base cost
	return p_base_cost
