## Genome-Aware Action Base
## Extend this instead of GOAPAction to get automatic weight genome support.
## Actions that extend this will automatically use optimized weights when available.
##
## Usage:
##   1. Change: extends GOAPAction
##      To:     extends GenomeAwareAction
##
##   2. Override _get_base_cost_key() to return the genome weight key
##
##   3. In _get_dynamic_cost(), use get_modifier() for dynamic adjustments
##
## Example:
##   func _get_base_cost_key() -> String:
##       return "cost_go_to_cover"
##
##   func _get_dynamic_cost(agent, blackboard, base) -> int:
##       var cost = base
##       if blackboard.get_var("melee_threat"):
##           cost = get_modifier(agent, "cost_cover_vs_melee", 100)
##       return cost
@tool
class_name GenomeAwareAction
extends GOAPAction

const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")


## Override this to specify which genome weight to use for base cost
## Return empty string to use the action's base_cost property
func _get_base_cost_key() -> String:
	return ""


## Gets the effective base cost, checking for genome override
func get_effective_base_cost(agent: Node) -> int:
	var key := _get_base_cost_key()
	if key.is_empty():
		return base_cost

	return WeightApplicatorClass.get_weight(agent, key, base_cost)


## Gets a modifier value from genome, with fallback
func get_modifier(agent: Node, key: String, default: Variant) -> Variant:
	return WeightApplicatorClass.get_weight(agent, key, default)


## Gets a threshold value from genome
func get_threshold(agent: Node, key: String, default: float) -> float:
	return WeightApplicatorClass.get_weight(agent, key, default)


## Override GOAPAction's get_cost to use genome-aware base cost
## Note: This requires GOAPAction to call _get_dynamic_cost with the correct base
func _get_dynamic_cost(agent: Node, blackboard: Blackboard, base: int) -> int:
	# Default implementation just returns the genome-aware base cost
	return get_effective_base_cost(agent)
