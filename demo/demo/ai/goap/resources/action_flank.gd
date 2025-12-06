## Flank action with dynamic cost based on target cover status and agent health
## Flanking is useful when target is in cover, less so when they're exposed
## Low health makes flanking risky
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost := WeightApplicatorClass.get_weight(agent, "cost_flank", GOAPConfigClass.COST_FLANK) as int

	# Target in cover makes flanking valuable
	var target_in_cover: bool = blackboard.get_var(&"target_in_cover", false)
	if target_in_cover:
		cost += WeightApplicatorClass.get_weight(agent, "cost_flank_target_in_cover", GOAPConfigClass.COST_FLANK_TARGET_IN_COVER) as int
	else:
		# Target exposed - flanking less necessary
		cost += WeightApplicatorClass.get_weight(agent, "cost_flank_target_exposed", GOAPConfigClass.COST_FLANK_TARGET_EXPOSED) as int

	# Low health makes flanking risky
	var health_ratio: float = blackboard.get_var(&"health_ratio", 1.0)
	if health_ratio < 0.5:
		cost += WeightApplicatorClass.get_weight(agent, "cost_flank_low_health", GOAPConfigClass.COST_FLANK_LOW_HEALTH) as int

	return maxi(1, cost)
