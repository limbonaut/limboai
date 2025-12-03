## GoToCover action with dynamic cost based on threat level
## Lower cost when under threat = higher priority for seeking cover
extends GOAPAction


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, p_base_cost: int) -> int:
	# If under threat, dramatically reduce cost to prioritize cover-seeking
	var under_threat = blackboard.get_var(&"under_threat", false)
	if under_threat:
		# Return minimum cost (1) when under threat to make this the highest priority
		return 1
	# Otherwise use base cost
	return p_base_cost
