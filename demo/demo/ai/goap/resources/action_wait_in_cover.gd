## Wait In Cover action with dynamic cost based on resources and combat time
## Expensive when low on ammo/health (should reload/heal), also when combat drags on
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost: int = p_base_cost

	# More expensive when low ammo (encourage reloading)
	var ammo_efficiency: float = blackboard.get_var(&"ammo_efficiency", 1.0)
	if ammo_efficiency < 0.3:
		cost += WeightApplicatorClass.get_weight(agent, "cost_wait_low_ammo", GOAPConfigClass.COST_WAIT_LOW_AMMO) as int

	# More expensive when low health (encourage healing)
	var health_ratio: float = blackboard.get_var(&"health_ratio", 1.0)
	if health_ratio < 0.4:
		cost += WeightApplicatorClass.get_weight(agent, "cost_wait_low_health", GOAPConfigClass.COST_WAIT_LOW_HEALTH) as int

	# More expensive as combat drags on
	var time_in_combat: float = blackboard.get_var(&"time_in_combat", 0.0)
	var engagement_window: float = WeightApplicatorClass.get_weight(agent, "engagement_window", GOAPConfigClass.ENGAGEMENT_WINDOW)
	if time_in_combat > engagement_window * 2.0:
		cost += WeightApplicatorClass.get_weight(agent, "cost_wait_long_combat", GOAPConfigClass.COST_WAIT_LONG_COMBAT) as int

	return maxi(1, cost)
