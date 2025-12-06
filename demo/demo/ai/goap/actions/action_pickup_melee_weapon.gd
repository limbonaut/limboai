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
##
## Supports WeightGenome optimization:
## - mod_preferred_weapon_bonus: Bonus for preferred weapon type (negative)
## - mod_nonpreferred_weapon_penalty: Penalty for non-preferred (positive)
## - mod_close_pickup_bonus: Bonus when pickup is close (negative)
## - mod_far_pickup_penalty: Penalty when pickup is far (positive)
## - mod_counter_weapon_bonus: Bonus when countering enemy weapon (negative)
## - mod_same_weapon_penalty: Penalty for same weapon matchup (positive)
## - mod_enemy_close_melee_bonus: Melee bonus when enemy is close (negative)
## - mod_enemy_far_melee_penalty: Melee penalty when enemy is far (positive)
## - mod_no_ammo_melee_bonus: Melee bonus when no ammo available (negative)

const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")


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
				cost += WeightApplicatorClass.get_weight(agent, "mod_preferred_weapon_bonus", -3)
			CombatComponentClass.CombatMode.RANGED:
				cost += WeightApplicatorClass.get_weight(agent, "mod_nonpreferred_weapon_penalty", 5)

	# === 2. DISTANCE TO PICKUP ===
	# Closer pickups are more attractive (less travel time = lower cost)
	var melee_pickup: Node2D = blackboard.get_var(&"melee_weapon_pickup", null)
	if is_instance_valid(melee_pickup) and is_instance_valid(agent):
		var dist: float = agent.global_position.distance_to(melee_pickup.global_position)
		# Normalize distance: 0-200 = close, 200-500 = medium, 500+ = far
		if dist < 200.0:
			cost += WeightApplicatorClass.get_weight(agent, "mod_close_pickup_bonus", -2)
		elif dist > 500.0:
			cost += WeightApplicatorClass.get_weight(agent, "mod_far_pickup_penalty", 3)

	# === 3. COUNTER-PLAY: ENEMY WEAPON TYPE ===
	# Melee is good against ranged enemies (can close gap and overwhelm)
	var enemy_has_ranged: bool = blackboard.get_var(&"enemy_has_ranged_weapon", false)
	var enemy_has_melee: bool = blackboard.get_var(&"enemy_has_melee_weapon", false)

	if enemy_has_ranged and not enemy_has_melee:
		cost += WeightApplicatorClass.get_weight(agent, "mod_counter_weapon_bonus", -2)
	elif enemy_has_melee and not enemy_has_ranged:
		cost += WeightApplicatorClass.get_weight(agent, "mod_same_weapon_penalty", 1)

	# === 4. TACTICAL SITUATION ===
	# If enemy is already close, melee is immediately useful
	var target: Node2D = blackboard.get_var(&"target", null)
	var too_close_threshold: float = WeightApplicatorClass.get_weight(agent, "too_close_threshold", GOAPConfigClass.TOO_CLOSE_THRESHOLD)
	if is_instance_valid(target) and is_instance_valid(agent):
		var enemy_dist: float = agent.global_position.distance_to(target.global_position)
		if enemy_dist < too_close_threshold:
			cost += WeightApplicatorClass.get_weight(agent, "mod_enemy_close_melee_bonus", -3)
		elif enemy_dist > GOAPConfigClass.SHOOTING_RANGE * 0.8:
			cost += WeightApplicatorClass.get_weight(agent, "mod_enemy_far_melee_penalty", 2)

	# === 5. RESOURCE CONSIDERATION ===
	# If no ammo is available, melee becomes more attractive (ranged useless without ammo)
	var ammo_available: bool = blackboard.get_var(&"ammo_available", true)
	if not ammo_available:
		cost += WeightApplicatorClass.get_weight(agent, "mod_no_ammo_melee_bonus", -2)

	# Clamp to reasonable range (minimum 1)
	return maxi(1, cost)
