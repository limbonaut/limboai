## Take Cover action with dynamic cost based on threat and health
## Expensive in melee range (escape first!), cheaper when exposed or low health
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost := WeightApplicatorClass.get_weight(agent, "cost_take_cover", GOAPConfigClass.COST_TAKE_COVER) as int

	# More expensive in melee range (escape first!)
	var melee_threat: bool = blackboard.get_var(&"melee_threat", false)
	if melee_threat:
		cost += WeightApplicatorClass.get_weight(agent, "cost_take_cover_melee_range", GOAPConfigClass.COST_TAKE_COVER_MELEE_RANGE) as int

	# Cheaper when exposed with ranged threat
	var ranged_threat: bool = blackboard.get_var(&"ranged_threat", false)
	var in_cover: bool = blackboard.get_var(&"in_cover", false)
	if ranged_threat and not in_cover:
		cost += WeightApplicatorClass.get_weight(agent, "cost_take_cover_exposed", GOAPConfigClass.COST_TAKE_COVER_EXPOSED) as int

	# Cheaper when health is low
	var health_ratio: float = blackboard.get_var(&"health_ratio", 1.0)
	if health_ratio < 0.4:
		cost += WeightApplicatorClass.get_weight(agent, "cost_take_cover_low_health", GOAPConfigClass.COST_TAKE_COVER_LOW_HEALTH) as int

	return maxi(1, cost)
