@tool
extends GOAPAction
## Pickup ranged weapon action with dynamic cost based on agent's combat mode.
## Ranged agents get lower cost, melee agents get higher cost.

const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, base: int) -> int:
	if not is_instance_valid(agent):
		return base

	# Check agent's preferred combat mode
	if agent.has_node("CombatComponent"):
		var combat: CombatComponentClass = agent.get_node("CombatComponent")
		match combat.preferred_mode:
			CombatComponentClass.CombatMode.RANGED:
				return 1  # Low cost - ranged agents want ranged weapons
			CombatComponentClass.CombatMode.MELEE:
				return 10  # High cost - melee agents avoid ranged weapons

	return base
