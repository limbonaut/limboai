extends RefCounted
## Adjusts action costs based on threat level.
## Balances immediate survival with critical needs like health and ammo.

## Actions that are considered offensive/risky when under threat
const OFFENSIVE_ACTIONS := ["Shoot", "Melee", "GoToTarget", "GoToWeapon"]

## Actions that become cheaper when under threat (defensive)
const DEFENSIVE_ACTIONS := ["GoToCover", "TakeCover", "Dodge"]

## Actions needed for survival that can override threat avoidance
const SURVIVAL_ACTIONS := ["GoToHealth", "Heal", "GoToAmmo", "Reload"]

## Health threshold where survival overrides threat response
const CRITICAL_HEALTH := 30
const LOW_HEALTH := 50


func modify(action_name: String, base_cost: int, ctx: Dictionary) -> int:
	var under_threat: bool = ctx.get("under_threat", false)
	var in_cover: bool = ctx.get("in_cover", false)
	var enemy_attacking: bool = ctx.get("enemy_attacking", false)
	var health: int = ctx.get("health", 100)
	var has_ammo: bool = ctx.get("has_ammo", false)

	# When under threat and not in cover - PRIORITY: GET TO COVER
	if under_threat and not in_cover:
		# Critical health: survival trumps threat avoidance
		if health < CRITICAL_HEALTH and action_name in SURVIVAL_ACTIONS:
			return 2  # Allow healing even under fire

		# Out of ammo: must resupply to fight back
		if not has_ammo and action_name in ["GoToAmmo", "Reload"]:
			return 5  # Higher cost than cover, but possible

		if action_name in OFFENSIVE_ACTIONS:
			# Make offensive actions very expensive - seek cover first!
			return 100
		if action_name in DEFENSIVE_ACTIONS:
			# Make defensive actions very cheap
			return 1

	# NOT under threat - defensive actions are wasteful
	if not under_threat:
		if action_name in DEFENSIVE_ACTIONS:
			# Don't randomly go to cover when safe
			return 50

	# In cover and safe moment: allow tactical moves
	if in_cover and not enemy_attacking:
		if action_name in SURVIVAL_ACTIONS:
			return base_cost  # No penalty for healing/resupply from cover

	# If in cover and threat is active, slight penalty to leaving cover
	if in_cover and enemy_attacking:
		if action_name == "LeaveCover":
			return base_cost + 5

	return base_cost


func get_modifier_name() -> String:
	return "Threat"
