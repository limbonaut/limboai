## GoToCover action with dynamic cost based on threat TYPE
## Cover is effective against ranged threats but USELESS against melee threats
## This guides the planner to choose appropriate defensive actions
##
## Supports WeightGenome optimization:
## - cost_go_to_cover: Base cost for this action
## - cost_cover_vs_melee: Cost when facing melee threat (high = avoid)
## - cost_cover_vs_ranged: Cost when facing ranged threat (low = prefer)
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var melee_threat = blackboard.get_var(&"melee_threat", false)
	var ranged_threat = blackboard.get_var(&"ranged_threat", false)

	# Cover is USELESS against melee - hiding behind a wall won't stop a sword
	# Make cost very high so planner will choose retreat instead
	if melee_threat and not ranged_threat:
		# Use genome weight if available, otherwise use optimized config constant
		return WeightApplicatorClass.get_weight(agent, "cost_cover_vs_melee", GOAPConfigClass.COST_COVER_VS_MELEE)

	# Cover is EFFECTIVE against ranged - prioritize it
	if ranged_threat:
		# Use genome weight if available, otherwise use optimized config constant
		return WeightApplicatorClass.get_weight(agent, "cost_cover_vs_ranged", GOAPConfigClass.COST_COVER_VS_RANGED)

	# General threat without specific type - use moderate priority
	var under_threat = blackboard.get_var(&"under_threat", false)
	if under_threat:
		return 2

	# No threat - use optimized config constant
	return WeightApplicatorClass.get_weight(agent, "cost_go_to_cover", GOAPConfigClass.COST_GO_TO_COVER)
