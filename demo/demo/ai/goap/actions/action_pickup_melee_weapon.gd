@tool
extends GOAPAction
## Pickup melee weapon action with tactical cost calculation.
## Considers: agent preference, distance to pickup, enemy weapon type, and tactical situation.
##
## Cost Factors:
## - Base preference: Melee-preferred agents get lower cost
## - Distance: Closer pickups get cost reduction
## - Counter-play: If enemy has ranged, melee is good for closing gap
## - Tactical: If enemy is close, melee is more immediately useful

const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")


func _get_dynamic_cost(agent: Node, blackboard: Blackboard, base: int) -> int:
	if not is_instance_valid(agent):
		return base

	var cost := base

	# === 1. AGENT PREFERENCE ===
	# Melee-preferred agents strongly prefer melee weapons
	if agent.has_node("CombatComponent"):
		var combat: CombatComponentClass = agent.get_node("CombatComponent")
		match combat.preferred_mode:
			CombatComponentClass.CombatMode.MELEE:
				cost -= 3  # Strong preference bonus
			CombatComponentClass.CombatMode.RANGED:
				cost += 5  # Penalty for non-preferred

	# === 2. DISTANCE TO PICKUP ===
	# Closer pickups are more attractive (less travel time = lower cost)
	var melee_pickup: Node2D = blackboard.get_var(&"melee_weapon_pickup", null)
	if is_instance_valid(melee_pickup) and is_instance_valid(agent):
		var dist: float = agent.global_position.distance_to(melee_pickup.global_position)
		# Normalize distance: 0-200 = close, 200-500 = medium, 500+ = far
		if dist < 200.0:
			cost -= 2  # Very close - easy grab
		elif dist > 500.0:
			cost += 3  # Far away - significant travel

	# === 3. COUNTER-PLAY: ENEMY WEAPON TYPE ===
	# Melee is good against ranged enemies (can close gap and overwhelm)
	var enemy_has_ranged: bool = blackboard.get_var(&"enemy_has_ranged_weapon", false)
	var enemy_has_melee: bool = blackboard.get_var(&"enemy_has_melee_weapon", false)

	if enemy_has_ranged and not enemy_has_melee:
		cost -= 2  # Melee counters ranged (close gap, deny kiting)
	elif enemy_has_melee and not enemy_has_ranged:
		cost += 1  # Melee vs melee is neutral, slight penalty (ranged might be safer)

	# === 4. TACTICAL SITUATION ===
	# If enemy is already close, melee is immediately useful
	var target: Node2D = blackboard.get_var(&"target", null)
	if is_instance_valid(target) and is_instance_valid(agent):
		var enemy_dist: float = agent.global_position.distance_to(target.global_position)
		if enemy_dist < GOAPConfigClass.TOO_CLOSE_THRESHOLD:
			cost -= 3  # Enemy is close - melee is great!
		elif enemy_dist > GOAPConfigClass.SHOOTING_RANGE * 0.8:
			cost += 2  # Enemy is far - melee requires closing distance

	# === 5. RESOURCE CONSIDERATION ===
	# If no ammo is available, melee becomes more attractive (ranged useless without ammo)
	var ammo_available: bool = blackboard.get_var(&"ammo_available", true)
	if not ammo_available:
		cost -= 2  # Melee doesn't need ammo

	# Clamp to reasonable range (minimum 1)
	return maxi(1, cost)
