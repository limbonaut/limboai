## Melee Attack action with dynamic cost based on enemy weapon and health
## Bonus when enemy has ranged (counter), penalty when low health (risky)
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost := WeightApplicatorClass.get_weight(agent, "cost_melee_attack", GOAPConfigClass.COST_MELEE_ATTACK) as int

	# Bonus when enemy has ranged (counter weapon matchup)
	var target_has_ranged: bool = blackboard.get_var(&"target_has_ranged", false)
	if target_has_ranged:
		cost += WeightApplicatorClass.get_weight(agent, "cost_melee_vs_ranged", GOAPConfigClass.COST_MELEE_VS_RANGED) as int

	# Penalty for melee vs melee (fair fight, no advantage)
	var target_has_melee: bool = blackboard.get_var(&"target_has_melee", false)
	if target_has_melee:
		cost += WeightApplicatorClass.get_weight(agent, "cost_melee_vs_melee", GOAPConfigClass.COST_MELEE_VS_MELEE) as int

	# Penalty when low health (risky to get close)
	var health_ratio: float = blackboard.get_var(&"health_ratio", 1.0)
	if health_ratio < 0.4:
		cost += WeightApplicatorClass.get_weight(agent, "cost_melee_low_health", GOAPConfigClass.COST_MELEE_LOW_HEALTH) as int

	return maxi(1, cost)
