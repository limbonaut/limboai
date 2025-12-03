extends RefCounted
## Adjusts action costs based on resource availability (ammo, etc.)
## Encourages resource gathering when running low.

## Actions related to resource gathering
const AMMO_ACTIONS := ["GoToAmmo", "Reload"]
const WEAPON_ACTIONS := ["GoToWeapon", "PickupWeapon"]

## Ammo thresholds
const LOW_AMMO := 3
const NO_AMMO := 0


func modify(action_name: String, base_cost: int, ctx: Dictionary) -> int:
	var has_ammo: bool = ctx.get("has_ammo", false)
	var ammo_count: int = ctx.get("ammo_count", 0)
	var under_threat: bool = ctx.get("under_threat", false)

	# No ammo: Shooting is impossible, ammo gathering is critical
	if not has_ammo:
		if action_name == "Shoot":
			return 1000  # Effectively impossible (will fail preconditions anyway)
		if action_name in AMMO_ACTIONS:
			# Ammo is critical but don't prioritize over immediate survival
			if under_threat:
				return 5
			return 2
		if action_name == "Melee":
			# Melee becomes more attractive when out of ammo
			return maxi(1, base_cost - 3)

	# Low ammo: start thinking about resupply
	if ammo_count <= LOW_AMMO:
		if action_name in AMMO_ACTIONS:
			return 3  # Moderate priority

	return base_cost


func get_modifier_name() -> String:
	return "Resource"
