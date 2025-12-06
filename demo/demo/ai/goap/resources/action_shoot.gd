## Shoot action with dynamic cost based on ammo and target position
## Penalty for last ammo, more expensive when target is in cover
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var cost := WeightApplicatorClass.get_weight(agent, "cost_shoot", GOAPConfigClass.COST_SHOOT) as int

	# Penalty for using final shot
	var ammo: int = blackboard.get_var(&"ammo", 0)
	if ammo == 1:
		cost += WeightApplicatorClass.get_weight(agent, "cost_shoot_last_ammo", GOAPConfigClass.COST_SHOOT_LAST_AMMO) as int

	# More expensive when target is in cover
	var target_in_cover: bool = blackboard.get_var(&"target_in_cover", false)
	if target_in_cover:
		cost += WeightApplicatorClass.get_weight(agent, "cost_shoot_target_in_cover", GOAPConfigClass.COST_SHOOT_TARGET_IN_COVER) as int
	else:
		# Bonus when target is exposed/close
		cost += WeightApplicatorClass.get_weight(agent, "cost_shoot_high_accuracy", GOAPConfigClass.COST_SHOOT_HIGH_ACCURACY) as int

	return maxi(1, cost)
