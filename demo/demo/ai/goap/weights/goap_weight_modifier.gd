extends RefCounted
## Base class for GOAP weight modifiers.
## Override modify() to adjust action costs based on context.

## Returns modified cost for an action based on context.
## [param action_name]: Name of the GOAP action being evaluated
## [param base_cost]: Current cost (may have been modified by previous modifiers)
## [param ctx]: Context dictionary with agent, blackboard, and any cached data
## Returns: Modified cost (return base_cost if no modification needed)
func modify(action_name: String, base_cost: int, ctx: Dictionary) -> int:
	return base_cost


## Optional: Override to provide a display name for debugging
func get_modifier_name() -> String:
	return "BaseModifier"
