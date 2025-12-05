@tool
extends GOAPAction
## Pickup melee weapon action with dynamic cost based on agent's combat mode.
## Melee agents get lower cost, ranged agents get higher cost.

const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, base: int) -> int:
	if not is_instance_valid(agent):
		return base

	# Check agent's preferred combat mode
	if agent.has_node("CombatComponent"):
		var combat: CombatComponentClass = agent.get_node("CombatComponent")
		match combat.preferred_mode:
			CombatComponentClass.CombatMode.MELEE:
				return 1  # Low cost - melee agents want melee weapons
			CombatComponentClass.CombatMode.RANGED:
				return 10  # High cost - ranged agents avoid melee weapons

	return base
