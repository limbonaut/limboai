## RetreatRanged action with dynamic cost based on threat type
## Retreat is the CORRECT response to melee threats for ranged agents
## Makes retreat very cheap when facing melee, so planner prefers it over cover
##
## Supports WeightGenome optimization:
## - cost_retreat: Base cost for this action
## - cost_retreat_vs_melee: Cost when facing melee threat (low = prefer)
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var melee_threat = blackboard.get_var(&"melee_threat", false)
	var enemy_has_melee = blackboard.get_var(&"enemy_has_melee_weapon", false)

	# Retreat is the RIGHT choice against melee - make it very cheap
	# This ensures the planner picks retreat over cover when facing melee
	if melee_threat or enemy_has_melee:
		# Use genome weight if available, otherwise use optimized config constant
		return WeightApplicatorClass.get_weight(agent, "cost_retreat_vs_melee", GOAPConfigClass.COST_RETREAT_VS_MELEE)

	# Otherwise use optimized config constant
	return WeightApplicatorClass.get_weight(agent, "cost_retreat", GOAPConfigClass.COST_RETREAT)
