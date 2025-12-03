extends RefCounted
## Orchestrates weight calculation by applying modifiers in order.
## Each modifier can adjust action costs based on environmental context.

## List of modifiers applied in order (first to last)
var modifiers: Array = []  # Array of GOAPWeightModifier

## Cached context dictionary, reused across calculations
var _context: Dictionary = {}


## Adds a modifier to the chain
func add_modifier(modifier: RefCounted) -> void:
	modifiers.append(modifier)


## Removes a modifier from the chain
func remove_modifier(modifier: RefCounted) -> void:
	modifiers.erase(modifier)


## Clears all modifiers
func clear_modifiers() -> void:
	modifiers.clear()


## Calculates the final cost for an action by applying all modifiers
## [param action_name]: Name of the GOAP action
## [param base_cost]: Original cost from the action resource
## [param agent]: The agent node
## [param blackboard]: The blackboard with current state
## Returns: Final modified cost
func calculate_cost(action_name: String, base_cost: int, agent: Node, blackboard: Blackboard) -> int:
	# Build context once per calculation
	_context.clear()
	_context["agent"] = agent
	_context["blackboard"] = blackboard

	# Cache commonly accessed blackboard values
	_context["health"] = blackboard.get_var(&"health", 100)
	_context["low_health"] = blackboard.get_var(&"low_health", false)
	_context["under_threat"] = blackboard.get_var(&"under_threat", false)
	_context["in_cover"] = blackboard.get_var(&"in_cover", false)
	_context["has_ammo"] = blackboard.get_var(&"has_ammo", false)
	_context["ammo_count"] = blackboard.get_var(&"ammo_count", 0)
	_context["target_visible"] = blackboard.get_var(&"target_visible", false)
	_context["enemy_attacking"] = blackboard.get_var(&"enemy_attacking", false)

	# Apply each modifier in order
	var cost := base_cost
	for modifier in modifiers:
		cost = modifier.modify(action_name, cost, _context)

	return cost


## Returns debug info about how cost was calculated (useful for UI/debugging)
func get_cost_breakdown(action_name: String, base_cost: int, agent: Node, blackboard: Blackboard) -> String:
	_context.clear()
	_context["agent"] = agent
	_context["blackboard"] = blackboard
	_context["health"] = blackboard.get_var(&"health", 100)
	_context["low_health"] = blackboard.get_var(&"low_health", false)
	_context["under_threat"] = blackboard.get_var(&"under_threat", false)
	_context["in_cover"] = blackboard.get_var(&"in_cover", false)
	_context["has_ammo"] = blackboard.get_var(&"has_ammo", false)
	_context["ammo_count"] = blackboard.get_var(&"ammo_count", 0)
	_context["target_visible"] = blackboard.get_var(&"target_visible", false)
	_context["enemy_attacking"] = blackboard.get_var(&"enemy_attacking", false)

	var breakdown := "%s: base=%d" % [action_name, base_cost]
	var cost := base_cost

	for modifier in modifiers:
		var new_cost: int = modifier.modify(action_name, cost, _context)
		if new_cost != cost:
			breakdown += " -> %s=%d" % [modifier.get_modifier_name(), new_cost]
			cost = new_cost

	breakdown += " (final=%d)" % cost
	return breakdown
