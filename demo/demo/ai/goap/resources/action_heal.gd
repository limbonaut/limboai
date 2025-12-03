## Heal action - uses layered weight system for dynamic cost
extends GOAPAction


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var calculator = blackboard.get_var(&"weight_calculator", null)
	if calculator and calculator.has_method("calculate_cost"):
		return calculator.calculate_cost(action_name, p_base_cost, agent, blackboard)
	return p_base_cost
