extends RefCounted
## Adjusts action costs based on health status.
## Makes healing a priority when health is low.

## Actions related to healing
const HEALING_ACTIONS := ["GoToHealth", "Heal"]

## Health thresholds
const CRITICAL_HEALTH := 30
const LOW_HEALTH := 50


func modify(action_name: String, base_cost: int, ctx: Dictionary) -> int:
	var health: int = ctx.get("health", 100)
	var low_health: bool = ctx.get("low_health", false)
	var under_threat: bool = ctx.get("under_threat", false)

	# Critical health: healing becomes top priority
	if health < CRITICAL_HEALTH:
		if action_name in HEALING_ACTIONS:
			return 1  # Highest priority
		# All other actions become more expensive
		return base_cost + 10

	# Low health and safe: prioritize healing
	if low_health and not under_threat:
		if action_name in HEALING_ACTIONS:
			return 2  # High priority when safe
		# Offensive actions get a slight penalty
		return base_cost + 3

	# Low health but under threat: cover first, then heal if possible
	if low_health and under_threat:
		if action_name in HEALING_ACTIONS:
			return 3  # Still prioritize, but after immediate threat

	return base_cost


func get_modifier_name() -> String:
	return "Health"
