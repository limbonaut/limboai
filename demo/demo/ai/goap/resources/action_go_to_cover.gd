## GoToCover action with dynamic cost based on threat TYPE
## Cover is effective against ranged threats but USELESS against melee threats
## This guides the planner to choose appropriate defensive actions
extends GOAPAction


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	var melee_threat = blackboard.get_var(&"melee_threat", false)
	var ranged_threat = blackboard.get_var(&"ranged_threat", false)

	# Cover is USELESS against melee - hiding behind a wall won't stop a sword
	# Make cost very high so planner will choose retreat instead
	if melee_threat and not ranged_threat:
		return 100  # Very high cost - discourage cover against melee

	# Cover is EFFECTIVE against ranged - prioritize it
	if ranged_threat:
		return 1  # Minimum cost - cover is the right choice

	# General threat without specific type - use moderate priority
	var under_threat = blackboard.get_var(&"under_threat", false)
	if under_threat:
		return 2

	# No threat - use base cost
	return p_base_cost
