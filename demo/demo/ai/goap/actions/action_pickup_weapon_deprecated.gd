@tool
extends GOAPAction
## DEPRECATED: Use action_pickup_melee_weapon or action_pickup_ranged_weapon instead.
## This generic action remains for backward compatibility but has high cost to discourage use.
##
## Problems with this action:
## - Claims to produce BOTH melee and ranged weapon effects
## - No tactical cost calculation
## - Planner cannot distinguish between weapon types

const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")

var _warned := false


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, base: int) -> int:
	# Log deprecation warning once
	if not _warned:
		push_warning("DEPRECATED: PickUpWeapon action is deprecated. Use PickUpMeleeWeapon or PickUpRangedWeapon for tactical planning.")
		_warned = true

	# Return high cost to discourage use when specific actions are available
	return 100
