## Leave Cover action with dynamic cost based on threat type and health
## Expensive with ranged threat, cheaper to escape melee or when healthy
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost := WeightApplicatorClass.get_weight(agent, "cost_leave_cover", GOAPConfigClass.COST_LEAVE_COVER) as int

	# More expensive with active ranged threat
	var ranged_threat: bool = blackboard.get_var(&"ranged_threat", false)
	if ranged_threat:
		cost += WeightApplicatorClass.get_weight(agent, "cost_leave_cover_ranged_threat", GOAPConfigClass.COST_LEAVE_COVER_RANGED_THREAT) as int

	# Cheaper to escape melee (cover won't help)
	var melee_threat: bool = blackboard.get_var(&"melee_threat", false)
	if melee_threat:
		cost += WeightApplicatorClass.get_weight(agent, "cost_leave_cover_melee_threat", GOAPConfigClass.COST_LEAVE_COVER_MELEE_THREAT) as int

	# Cheaper when healthy (can take hits)
	var health_ratio: float = blackboard.get_var(&"health_ratio", 1.0)
	if health_ratio > 0.7:
		cost += WeightApplicatorClass.get_weight(agent, "cost_leave_cover_high_health", GOAPConfigClass.COST_LEAVE_COVER_HIGH_HEALTH) as int

	return maxi(1, cost)
