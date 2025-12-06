## Approach Target action with dynamic cost based on enemy and agent state
## More expensive vs entrenched enemy, cheaper vs wounded target
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost := WeightApplicatorClass.get_weight(agent, "cost_approach_target", GOAPConfigClass.COST_APPROACH_TARGET) as int

	# More expensive vs entrenched enemy
	var target_in_cover: bool = blackboard.get_var(&"target_in_cover", false)
	if target_in_cover:
		cost += WeightApplicatorClass.get_weight(agent, "cost_approach_target_in_cover", GOAPConfigClass.COST_APPROACH_TARGET_IN_COVER) as int

	# Cheaper vs wounded target
	var target_health_ratio: float = blackboard.get_var(&"target_health_ratio", 1.0)
	if target_health_ratio < 0.5:
		cost += WeightApplicatorClass.get_weight(agent, "cost_approach_wounded_target", GOAPConfigClass.COST_APPROACH_WOUNDED_TARGET) as int

	# More expensive if recently took damage
	var recent_damage: bool = blackboard.get_var(&"recent_damage", false)
	if recent_damage:
		cost += WeightApplicatorClass.get_weight(agent, "cost_approach_recent_damage", GOAPConfigClass.COST_APPROACH_RECENT_DAMAGE) as int

	return maxi(1, cost)
