## Close Gap action with dynamic cost based on enemy type and agent state
## Riskier vs ranged enemies, cheaper when target is wounded
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost: int = p_base_cost

	# More expensive vs ranged enemy (risky approach)
	var ranged_threat: bool = blackboard.get_var(&"ranged_threat", false)
	if ranged_threat:
		cost += WeightApplicatorClass.get_weight(agent, "cost_close_vs_ranged", GOAPConfigClass.COST_CLOSE_VS_RANGED) as int

	# More expensive when health is critical
	var health_ratio: float = blackboard.get_var(&"health_ratio", 1.0)
	if health_ratio < 0.3:
		cost += WeightApplicatorClass.get_weight(agent, "cost_close_low_health", GOAPConfigClass.COST_CLOSE_LOW_HEALTH) as int

	# Cheaper when target is wounded (finish them!)
	var target_health_ratio: float = blackboard.get_var(&"target_health_ratio", 1.0)
	if target_health_ratio < 0.5:
		cost += WeightApplicatorClass.get_weight(agent, "cost_close_target_wounded", GOAPConfigClass.COST_CLOSE_TARGET_WOUNDED) as int

	return maxi(1, cost)
