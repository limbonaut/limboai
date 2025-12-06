## Heal action with dynamic cost based on threat and health gap
## More expensive when actively threatened, scales by how much health is needed
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost: int = p_base_cost

	# More expensive when actively threatened
	var under_threat: bool = blackboard.get_var(&"under_threat", false)
	if under_threat:
		cost += WeightApplicatorClass.get_weight(agent, "cost_heal_during_threat", GOAPConfigClass.COST_HEAL_DURING_THREAT) as int

	# Scale by health gap (lower health = cheaper to heal)
	var health_ratio: float = blackboard.get_var(&"health_ratio", 1.0)
	var health_scale: float = WeightApplicatorClass.get_weight(agent, "cost_heal_health_scale", GOAPConfigClass.COST_HEAL_HEALTH_SCALE)
	var health_discount: int = int((1.0 - health_ratio) * health_scale * 5.0)  # Up to -5 cost when very low health
	cost -= health_discount

	# More expensive in active combat
	var time_in_combat: float = blackboard.get_var(&"time_in_combat", 0.0)
	if time_in_combat > 0.5:
		cost += WeightApplicatorClass.get_weight(agent, "cost_heal_combat_active", GOAPConfigClass.COST_HEAL_COMBAT_ACTIVE) as int

	return maxi(1, cost)
