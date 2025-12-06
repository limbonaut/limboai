@tool
extends GOAPAction
## Pickup ranged weapon action with tactical cost calculation.
## Considers: agent preference, distance to pickup, enemy weapon type, and tactical situation.
##
## Cost Factors:
## - Base preference: Ranged-preferred agents get lower cost
## - Distance: Closer pickups get cost reduction
## - Counter-play: If enemy has melee, ranged is good for kiting
## - Tactical: If enemy is far or cover is available, ranged is more useful
## - Resources: Requires ammo to be effective

const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, base: int) -> int:
	if not is_instance_valid(agent):
		return base

	var cost := base

	# === 1. AGENT PREFERENCE ===
	# Ranged-preferred agents strongly prefer ranged weapons
	if agent.has_node("CombatComponent"):
		var combat: CombatComponentClass = agent.get_node("CombatComponent")
		match combat.preferred_mode:
			CombatComponentClass.CombatMode.RANGED:
				cost -= 3  # Strong preference bonus
			CombatComponentClass.CombatMode.MELEE:
				cost += 5  # Penalty for non-preferred

	# === 2. DISTANCE TO PICKUP ===
	# Closer pickups are more attractive (less travel time = lower cost)
	var ranged_pickup: Node2D = blackboard.get_var(&"ranged_weapon_pickup", null)
	if is_instance_valid(ranged_pickup) and is_instance_valid(agent):
		var dist: float = agent.global_position.distance_to(ranged_pickup.global_position)
		# Normalize distance: 0-200 = close, 200-500 = medium, 500+ = far
		if dist < 200.0:
			cost -= 2  # Very close - easy grab
		elif dist > 500.0:
			cost += 3  # Far away - significant travel

	# === 3. COUNTER-PLAY: ENEMY WEAPON TYPE ===
	# Ranged is good against melee enemies (can kite and maintain distance)
	var enemy_has_ranged: bool = blackboard.get_var(&"enemy_has_ranged_weapon", false)
	var enemy_has_melee: bool = blackboard.get_var(&"enemy_has_melee_weapon", false)

	if enemy_has_melee and not enemy_has_ranged:
		cost -= 2  # Ranged counters melee (kite, maintain distance)
	elif enemy_has_ranged and not enemy_has_melee:
		cost += 1  # Ranged vs ranged is a cover/aim battle - neutral

	# === 4. TACTICAL SITUATION ===
	# If enemy is far, ranged is immediately useful
	# If cover is available, ranged synergizes well
	var target: Node2D = blackboard.get_var(&"target", null)
	if is_instance_valid(target) and is_instance_valid(agent):
		var enemy_dist: float = agent.global_position.distance_to(target.global_position)
		if enemy_dist > GOAPConfigClass.RETREAT_DISTANCE:
			cost -= 2  # Enemy is far - ranged is great!
		elif enemy_dist < GOAPConfigClass.TOO_CLOSE_THRESHOLD:
			cost += 3  # Enemy is too close - ranged is awkward

	# Cover synergy: ranged works better with cover
	var near_cover: bool = blackboard.get_var(&"near_cover", false)
	if near_cover:
		cost -= 1  # Cover available - ranged can use it effectively

	# === 5. RESOURCE CONSIDERATION ===
	# Ranged needs ammo to be effective
	var ammo_available: bool = blackboard.get_var(&"ammo_available", true)
	var has_ammo: bool = blackboard.get_var(&"has_ammo", false)

	if not ammo_available and not has_ammo:
		cost += 4  # No ammo and can't get any - ranged is nearly useless
	elif not has_ammo and ammo_available:
		cost += 1  # Will need to get ammo after pickup

	# === 6. HEALTH CONSIDERATION ===
	# Low health agents prefer ranged (safer, can fight from distance)
	var low_health: bool = blackboard.get_var(&"low_health", false)
	if low_health:
		cost -= 2  # Ranged is safer when low on health

	# Clamp to reasonable range (minimum 1)
	return maxi(1, cost)
