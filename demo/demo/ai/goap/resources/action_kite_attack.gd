## Kite Attack action with dynamic cost based on threat type and resources
## Kiting is ideal vs melee threats, expensive when low on ammo
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost: int = p_base_cost

	# Kiting is ideal vs melee threats
	var melee_threat: bool = blackboard.get_var(&"melee_threat", false)
	if melee_threat:
		cost += WeightApplicatorClass.get_weight(agent, "cost_kite_vs_melee", GOAPConfigClass.COST_KITE_VS_MELEE) as int

	# Low ammo makes kiting less viable
	var ammo_efficiency: float = blackboard.get_var(&"ammo_efficiency", 1.0)
	var ammo_threshold: float = WeightApplicatorClass.get_weight(agent, "ammo_scarcity_threshold", GOAPConfigClass.AMMO_SCARCITY_THRESHOLD)
	if ammo_efficiency < ammo_threshold:
		cost += WeightApplicatorClass.get_weight(agent, "cost_kite_low_ammo", GOAPConfigClass.COST_KITE_LOW_AMMO) as int

	# High health makes kiting more attractive (can trade)
	var health_ratio: float = blackboard.get_var(&"health_ratio", 1.0)
	if health_ratio > 0.7:
		cost += WeightApplicatorClass.get_weight(agent, "cost_kite_high_health", GOAPConfigClass.COST_KITE_HIGH_HEALTH) as int

	return maxi(1, cost)
