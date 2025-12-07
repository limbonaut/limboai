## Weight Applicator
## Applies WeightGenome values to GOAP agents and actions at runtime.
## This is the bridge between optimized weights and the actual GOAP system.
##
## Usage:
##   var applicator = WeightApplicator.new()
##   applicator.apply_to_agent(agent, genome)
##
## Or for global application:
##   WeightApplicator.set_global_genome(genome)
##   # All agents will use this genome via get_weight() calls
class_name WeightApplicator
extends RefCounted

const WeightGenomeClass = preload("res://demo/ai/goap/optimization/weight_genome.gd")

## Global genome for all agents (optional)
static var _global_genome: Resource = null

## Sets a global genome that all agents will use
static func set_global_genome(genome: Resource) -> void:
	_global_genome = genome
	print("WeightApplicator: Set global genome")


## Clears the global genome
static func clear_global_genome() -> void:
	_global_genome = null


## Gets the genome for an agent (checks agent meta, then global)
static func get_genome_for_agent(agent: Node) -> Resource:
	if agent and agent.has_meta("weight_genome"):
		return agent.get_meta("weight_genome")
	return _global_genome


## Gets a specific weight value for an agent
## Falls back to default values if no genome is set
static func get_weight(agent: Node, weight_name: String, default_value: Variant) -> Variant:
	var genome := get_genome_for_agent(agent)
	if not genome:
		return default_value

	# Map weight names to genome properties
	match weight_name:
		# Action base costs
		"cost_go_to_weapon":
			return genome.cost_go_to_weapon
		"cost_pickup_weapon":
			return genome.cost_pickup_weapon
		"cost_go_to_ammo":
			return genome.cost_go_to_ammo
		"cost_load_weapon":
			return genome.cost_load_weapon
		"cost_go_to_cover":
			return genome.cost_go_to_cover
		"cost_take_cover":
			return genome.cost_take_cover
		"cost_leave_cover":
			return genome.cost_leave_cover
		"cost_approach_target":
			return genome.cost_approach_target
		"cost_flank":
			return genome.cost_flank
		"cost_shoot":
			return genome.cost_shoot
		"cost_melee_attack":
			return genome.cost_melee_attack
		"cost_retreat":
			return genome.cost_retreat

		# Dynamic cost modifiers
		"mod_preferred_weapon_bonus":
			return genome.mod_preferred_weapon_bonus
		"mod_nonpreferred_weapon_penalty":
			return genome.mod_nonpreferred_weapon_penalty
		"mod_close_pickup_bonus":
			return genome.mod_close_pickup_bonus
		"mod_far_pickup_penalty":
			return genome.mod_far_pickup_penalty
		"mod_counter_weapon_bonus":
			return genome.mod_counter_weapon_bonus
		"mod_same_weapon_penalty":
			return genome.mod_same_weapon_penalty
		"mod_enemy_close_melee_bonus":
			return genome.mod_enemy_close_melee_bonus
		"mod_enemy_far_melee_penalty":
			return genome.mod_enemy_far_melee_penalty
		"mod_no_ammo_melee_bonus":
			return genome.mod_no_ammo_melee_bonus

		# Threat costs
		"cost_cover_vs_melee":
			return genome.cost_cover_vs_melee
		"cost_cover_vs_ranged":
			return genome.cost_cover_vs_ranged
		"cost_retreat_vs_melee":
			return genome.cost_retreat_vs_melee
		"cost_ammo_with_melee":
			return genome.cost_ammo_with_melee

		# Context-aware action costs - Flank
		"cost_flank_target_in_cover":
			return genome.cost_flank_target_in_cover
		"cost_flank_target_exposed":
			return genome.cost_flank_target_exposed
		"cost_flank_low_health":
			return genome.cost_flank_low_health

		# Context-aware action costs - Kite
		"cost_kite_vs_melee":
			return genome.cost_kite_vs_melee
		"cost_kite_low_ammo":
			return genome.cost_kite_low_ammo
		"cost_kite_high_health":
			return genome.cost_kite_high_health

		# Context-aware action costs - Close Gap
		"cost_close_vs_ranged":
			return genome.cost_close_vs_ranged
		"cost_close_low_health":
			return genome.cost_close_low_health
		"cost_close_target_wounded":
			return genome.cost_close_target_wounded

		# Context-aware action costs - Approach
		"cost_approach_target_in_cover":
			return genome.cost_approach_target_in_cover
		"cost_approach_wounded_target":
			return genome.cost_approach_wounded_target
		"cost_approach_recent_damage":
			return genome.cost_approach_recent_damage

		# Context-aware action costs - Take Cover
		"cost_take_cover_melee_range":
			return genome.cost_take_cover_melee_range
		"cost_take_cover_exposed":
			return genome.cost_take_cover_exposed
		"cost_take_cover_low_health":
			return genome.cost_take_cover_low_health

		# Context-aware action costs - Leave Cover
		"cost_leave_cover_ranged_threat":
			return genome.cost_leave_cover_ranged_threat
		"cost_leave_cover_melee_threat":
			return genome.cost_leave_cover_melee_threat
		"cost_leave_cover_high_health":
			return genome.cost_leave_cover_high_health

		# Context-aware action costs - Wait In Cover
		"cost_wait_low_ammo":
			return genome.cost_wait_low_ammo
		"cost_wait_low_health":
			return genome.cost_wait_low_health
		"cost_wait_long_combat":
			return genome.cost_wait_long_combat

		# Context-aware action costs - Heal
		"cost_heal_during_threat":
			return genome.cost_heal_during_threat
		"cost_heal_health_scale":
			return genome.cost_heal_health_scale
		"cost_heal_combat_active":
			return genome.cost_heal_combat_active

		# Context-aware action costs - Shoot
		"cost_shoot_last_ammo":
			return genome.cost_shoot_last_ammo
		"cost_shoot_target_in_cover":
			return genome.cost_shoot_target_in_cover
		"cost_shoot_high_accuracy":
			return genome.cost_shoot_high_accuracy

		# Context-aware action costs - Melee
		"cost_melee_vs_ranged":
			return genome.cost_melee_vs_ranged
		"cost_melee_vs_melee":
			return genome.cost_melee_vs_melee
		"cost_melee_low_health":
			return genome.cost_melee_low_health

		# Global modifiers
		"health_cost_scaling":
			return genome.health_cost_scaling
		"ammo_scarcity_threshold":
			return genome.ammo_scarcity_threshold
		"engagement_window":
			return genome.engagement_window

		# Timing
		"goal_switch_cooldown":
			return genome.goal_switch_cooldown
		"defensive_goal_commitment":
			return genome.defensive_goal_commitment
		"attack_goal_commitment":
			return genome.attack_goal_commitment
		"defensive_timeout":
			return genome.defensive_timeout

		# Thresholds
		"retreat_distance":
			return genome.retreat_distance
		"close_gap_threshold":
			return genome.close_gap_threshold
		"too_close_threshold":
			return genome.too_close_threshold
		"low_health_threshold":
			return genome.low_health_threshold
		"healthy_threshold":
			return genome.healthy_threshold

		# Movement
		"ranged_move_speed":
			return genome.ranged_move_speed
		"melee_move_speed":
			return genome.melee_move_speed

		# Position Evaluation - Base Weights
		"pos_weight_threat_distance":
			return genome.pos_weight_threat_distance
		"pos_weight_ammo_proximity":
			return genome.pos_weight_ammo_proximity
		"pos_weight_health_proximity":
			return genome.pos_weight_health_proximity
		"pos_weight_cover_proximity":
			return genome.pos_weight_cover_proximity
		"pos_weight_center_proximity":
			return genome.pos_weight_center_proximity
		"pos_weight_strafe_preference":
			return genome.pos_weight_strafe_preference
		"pos_weight_los_to_target":
			return genome.pos_weight_los_to_target
		"pos_weight_speed_boost":
			return genome.pos_weight_speed_boost

		# Position Evaluation - Urgency
		"pos_ammo_urgency_scale":
			return genome.pos_ammo_urgency_scale
		"pos_health_urgency_scale":
			return genome.pos_health_urgency_scale
		"pos_low_ammo_threshold":
			return genome.pos_low_ammo_threshold
		"pos_low_health_threshold":
			return genome.pos_low_health_threshold

		# Position Evaluation - Movement
		"pos_step_distance":
			return genome.pos_step_distance
		"pos_wall_avoidance":
			return genome.pos_wall_avoidance
		"pos_safe_distance":
			return genome.pos_safe_distance
		"pos_danger_distance":
			return genome.pos_danger_distance

		# Position Evaluation - Mode Multipliers
		"pos_retreat_threat_mult":
			return genome.pos_retreat_threat_mult
		"pos_kite_strafe_mult":
			return genome.pos_kite_strafe_mult
		"pos_approach_speed_mult":
			return genome.pos_approach_speed_mult

		# Weapon-Type Multipliers - Ranged
		"ranged_threat_distance_mult":
			return genome.ranged_threat_distance_mult
		"ranged_ammo_proximity_mult":
			return genome.ranged_ammo_proximity_mult
		"ranged_health_proximity_mult":
			return genome.ranged_health_proximity_mult
		"ranged_cover_proximity_mult":
			return genome.ranged_cover_proximity_mult
		"ranged_center_proximity_mult":
			return genome.ranged_center_proximity_mult
		"ranged_strafe_preference_mult":
			return genome.ranged_strafe_preference_mult
		"ranged_safe_distance_mult":
			return genome.ranged_safe_distance_mult
		"ranged_danger_distance_mult":
			return genome.ranged_danger_distance_mult

		# Weapon-Type Multipliers - Melee
		"melee_threat_distance_mult":
			return genome.melee_threat_distance_mult
		"melee_ammo_proximity_mult":
			return genome.melee_ammo_proximity_mult
		"melee_health_proximity_mult":
			return genome.melee_health_proximity_mult
		"melee_cover_proximity_mult":
			return genome.melee_cover_proximity_mult
		"melee_center_proximity_mult":
			return genome.melee_center_proximity_mult
		"melee_strafe_preference_mult":
			return genome.melee_strafe_preference_mult
		"melee_safe_distance_mult":
			return genome.melee_safe_distance_mult
		"melee_danger_distance_mult":
			return genome.melee_danger_distance_mult

	# Unknown weight, return default
	return default_value


## Gets weapon-specific position evaluation weights for an agent
## This applies the weapon-type multipliers to base weights
static func get_weapon_adjusted_weight(agent: Node, base_weight_name: String, is_ranged: bool, default_value: float) -> float:
	var genome := get_genome_for_agent(agent)
	if not genome:
		return default_value

	var base_weight: float = get_weight(agent, base_weight_name, default_value)

	# Map base weights to their weapon-type multipliers
	var mult_name := ""
	match base_weight_name:
		"pos_weight_threat_distance":
			mult_name = "ranged_threat_distance_mult" if is_ranged else "melee_threat_distance_mult"
		"pos_weight_ammo_proximity":
			mult_name = "ranged_ammo_proximity_mult" if is_ranged else "melee_ammo_proximity_mult"
		"pos_weight_health_proximity":
			mult_name = "ranged_health_proximity_mult" if is_ranged else "melee_health_proximity_mult"
		"pos_weight_cover_proximity":
			mult_name = "ranged_cover_proximity_mult" if is_ranged else "melee_cover_proximity_mult"
		"pos_weight_center_proximity":
			mult_name = "ranged_center_proximity_mult" if is_ranged else "melee_center_proximity_mult"
		"pos_weight_strafe_preference":
			mult_name = "ranged_strafe_preference_mult" if is_ranged else "melee_strafe_preference_mult"
		"pos_safe_distance":
			mult_name = "ranged_safe_distance_mult" if is_ranged else "melee_safe_distance_mult"
		"pos_danger_distance":
			mult_name = "ranged_danger_distance_mult" if is_ranged else "melee_danger_distance_mult"
		_:
			return base_weight

	var multiplier: float = get_weight(agent, mult_name, 1.0)
	return base_weight * multiplier


## Applies a genome to an agent and all its components
static func apply_to_agent(agent: Node, genome: Resource) -> void:
	if not agent or not genome:
		return

	# Store genome on agent for lookup
	agent.set_meta("weight_genome", genome)

	# Apply to goal evaluator timing
	var goal_evaluator := agent.get_node_or_null("GoalEvaluator")
	if goal_evaluator:
		_apply_to_goal_evaluator(goal_evaluator, genome)

	# Apply to world state manager thresholds
	var world_state := agent.get_node_or_null("WorldStateManager")
	if world_state:
		_apply_to_world_state(world_state, genome)

	# Apply movement speeds
	var movement := agent.get_node_or_null("MovementComponent")
	if movement:
		_apply_to_movement(movement, genome)

	# Apply position evaluation weights
	var position_evaluator := agent.get_node_or_null("PositionEvaluator")
	if position_evaluator:
		_apply_to_position_evaluator(position_evaluator, genome)


static func _apply_to_goal_evaluator(evaluator: Node, genome: Resource) -> void:
	evaluator.set_meta("goal_switch_cooldown", genome.goal_switch_cooldown)
	evaluator.set_meta("defensive_goal_commitment", genome.defensive_goal_commitment)
	evaluator.set_meta("attack_goal_commitment", genome.attack_goal_commitment)
	evaluator.set_meta("defensive_timeout", genome.defensive_timeout)


static func _apply_to_world_state(world_state: Node, genome: Resource) -> void:
	world_state.set_meta("low_health_threshold", genome.low_health_threshold)
	world_state.set_meta("healthy_threshold", genome.healthy_threshold)
	world_state.set_meta("retreat_distance", genome.retreat_distance)
	world_state.set_meta("close_gap_threshold", genome.close_gap_threshold)
	world_state.set_meta("too_close_threshold", genome.too_close_threshold)


static func _apply_to_movement(movement: Node, genome: Resource) -> void:
	movement.set_meta("ranged_move_speed", genome.ranged_move_speed)
	movement.set_meta("melee_move_speed", genome.melee_move_speed)


static func _apply_to_position_evaluator(evaluator: Node, genome: Resource) -> void:
	# Base weights
	evaluator.weight_threat_distance = genome.pos_weight_threat_distance
	evaluator.weight_ammo_proximity = genome.pos_weight_ammo_proximity
	evaluator.weight_health_proximity = genome.pos_weight_health_proximity
	evaluator.weight_cover_proximity = genome.pos_weight_cover_proximity
	evaluator.weight_center_proximity = genome.pos_weight_center_proximity
	evaluator.weight_strafe_preference = genome.pos_weight_strafe_preference
	evaluator.weight_los_to_target = genome.pos_weight_los_to_target
	evaluator.weight_speed_boost_proximity = genome.pos_weight_speed_boost

	# Urgency scaling
	evaluator.ammo_urgency_scale = genome.pos_ammo_urgency_scale
	evaluator.health_urgency_scale = genome.pos_health_urgency_scale
	evaluator.low_ammo_threshold = genome.pos_low_ammo_threshold
	evaluator.low_health_threshold = genome.pos_low_health_threshold

	# Movement parameters
	evaluator.step_distance = genome.pos_step_distance
	evaluator.wall_avoidance = genome.pos_wall_avoidance
	evaluator.safe_distance = genome.pos_safe_distance
	evaluator.danger_distance = genome.pos_danger_distance

	# Mode multipliers
	evaluator.retreat_threat_mult = genome.pos_retreat_threat_mult
	evaluator.kite_strafe_mult = genome.pos_kite_strafe_mult
	evaluator.approach_speed_mult = genome.pos_approach_speed_mult

	# Weapon-type multipliers - Ranged
	evaluator.ranged_threat_distance_mult = genome.ranged_threat_distance_mult
	evaluator.ranged_ammo_proximity_mult = genome.ranged_ammo_proximity_mult
	evaluator.ranged_health_proximity_mult = genome.ranged_health_proximity_mult
	evaluator.ranged_cover_proximity_mult = genome.ranged_cover_proximity_mult
	evaluator.ranged_center_proximity_mult = genome.ranged_center_proximity_mult
	evaluator.ranged_strafe_preference_mult = genome.ranged_strafe_preference_mult
	evaluator.ranged_safe_distance_mult = genome.ranged_safe_distance_mult
	evaluator.ranged_danger_distance_mult = genome.ranged_danger_distance_mult

	# Weapon-type multipliers - Melee
	evaluator.melee_threat_distance_mult = genome.melee_threat_distance_mult
	evaluator.melee_ammo_proximity_mult = genome.melee_ammo_proximity_mult
	evaluator.melee_health_proximity_mult = genome.melee_health_proximity_mult
	evaluator.melee_cover_proximity_mult = genome.melee_cover_proximity_mult
	evaluator.melee_center_proximity_mult = genome.melee_center_proximity_mult
	evaluator.melee_strafe_preference_mult = genome.melee_strafe_preference_mult
	evaluator.melee_safe_distance_mult = genome.melee_safe_distance_mult
	evaluator.melee_danger_distance_mult = genome.melee_danger_distance_mult


## Exports a genome to GDScript code that can be pasted into goap_config.gd
static func export_to_gdscript(genome: Resource) -> String:
	var lines: Array[String] = []
	lines.append("## Optimized GOAP Weights")
	lines.append("## Generated by Evolutionary Optimizer")
	lines.append("## Generation: %d, Fitness: %.1f, Wins: %d, Losses: %d" % [
		genome.generation, genome.fitness, genome.wins, genome.losses
	])
	lines.append("")
	lines.append("# Action Base Costs")
	lines.append("const COST_GO_TO_WEAPON := %d" % genome.cost_go_to_weapon)
	lines.append("const COST_PICKUP_WEAPON := %d" % genome.cost_pickup_weapon)
	lines.append("const COST_GO_TO_AMMO := %d" % genome.cost_go_to_ammo)
	lines.append("const COST_LOAD_WEAPON := %d" % genome.cost_load_weapon)
	lines.append("const COST_GO_TO_COVER := %d" % genome.cost_go_to_cover)
	lines.append("const COST_TAKE_COVER := %d" % genome.cost_take_cover)
	lines.append("const COST_LEAVE_COVER := %d" % genome.cost_leave_cover)
	lines.append("const COST_APPROACH_TARGET := %d" % genome.cost_approach_target)
	lines.append("const COST_FLANK := %d" % genome.cost_flank)
	lines.append("const COST_SHOOT := %d" % genome.cost_shoot)
	lines.append("const COST_MELEE_ATTACK := %d" % genome.cost_melee_attack)
	lines.append("const COST_RETREAT := %d" % genome.cost_retreat)
	lines.append("")
	lines.append("# Dynamic Cost Modifiers")
	lines.append("const MOD_PREFERRED_WEAPON_BONUS := %d" % genome.mod_preferred_weapon_bonus)
	lines.append("const MOD_NONPREFERRED_WEAPON_PENALTY := %d" % genome.mod_nonpreferred_weapon_penalty)
	lines.append("const MOD_CLOSE_PICKUP_BONUS := %d" % genome.mod_close_pickup_bonus)
	lines.append("const MOD_FAR_PICKUP_PENALTY := %d" % genome.mod_far_pickup_penalty)
	lines.append("const MOD_COUNTER_WEAPON_BONUS := %d" % genome.mod_counter_weapon_bonus)
	lines.append("const MOD_SAME_WEAPON_PENALTY := %d" % genome.mod_same_weapon_penalty)
	lines.append("const MOD_ENEMY_CLOSE_MELEE_BONUS := %d" % genome.mod_enemy_close_melee_bonus)
	lines.append("const MOD_ENEMY_FAR_MELEE_PENALTY := %d" % genome.mod_enemy_far_melee_penalty)
	lines.append("const MOD_NO_AMMO_MELEE_BONUS := %d" % genome.mod_no_ammo_melee_bonus)
	lines.append("")
	lines.append("# Threat-Based Costs")
	lines.append("const COST_COVER_VS_MELEE := %d" % genome.cost_cover_vs_melee)
	lines.append("const COST_COVER_VS_RANGED := %d" % genome.cost_cover_vs_ranged)
	lines.append("const COST_RETREAT_VS_MELEE := %d" % genome.cost_retreat_vs_melee)
	lines.append("const COST_AMMO_WITH_MELEE := %d" % genome.cost_ammo_with_melee)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Flank")
	lines.append("const COST_FLANK_TARGET_IN_COVER := %d" % genome.cost_flank_target_in_cover)
	lines.append("const COST_FLANK_TARGET_EXPOSED := %d" % genome.cost_flank_target_exposed)
	lines.append("const COST_FLANK_LOW_HEALTH := %d" % genome.cost_flank_low_health)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Kite")
	lines.append("const COST_KITE_VS_MELEE := %d" % genome.cost_kite_vs_melee)
	lines.append("const COST_KITE_LOW_AMMO := %d" % genome.cost_kite_low_ammo)
	lines.append("const COST_KITE_HIGH_HEALTH := %d" % genome.cost_kite_high_health)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Close Gap")
	lines.append("const COST_CLOSE_VS_RANGED := %d" % genome.cost_close_vs_ranged)
	lines.append("const COST_CLOSE_LOW_HEALTH := %d" % genome.cost_close_low_health)
	lines.append("const COST_CLOSE_TARGET_WOUNDED := %d" % genome.cost_close_target_wounded)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Approach")
	lines.append("const COST_APPROACH_TARGET_IN_COVER := %d" % genome.cost_approach_target_in_cover)
	lines.append("const COST_APPROACH_WOUNDED_TARGET := %d" % genome.cost_approach_wounded_target)
	lines.append("const COST_APPROACH_RECENT_DAMAGE := %d" % genome.cost_approach_recent_damage)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Take Cover")
	lines.append("const COST_TAKE_COVER_MELEE_RANGE := %d" % genome.cost_take_cover_melee_range)
	lines.append("const COST_TAKE_COVER_EXPOSED := %d" % genome.cost_take_cover_exposed)
	lines.append("const COST_TAKE_COVER_LOW_HEALTH := %d" % genome.cost_take_cover_low_health)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Leave Cover")
	lines.append("const COST_LEAVE_COVER_RANGED_THREAT := %d" % genome.cost_leave_cover_ranged_threat)
	lines.append("const COST_LEAVE_COVER_MELEE_THREAT := %d" % genome.cost_leave_cover_melee_threat)
	lines.append("const COST_LEAVE_COVER_HIGH_HEALTH := %d" % genome.cost_leave_cover_high_health)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Wait In Cover")
	lines.append("const COST_WAIT_LOW_AMMO := %d" % genome.cost_wait_low_ammo)
	lines.append("const COST_WAIT_LOW_HEALTH := %d" % genome.cost_wait_low_health)
	lines.append("const COST_WAIT_LONG_COMBAT := %d" % genome.cost_wait_long_combat)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Heal")
	lines.append("const COST_HEAL_DURING_THREAT := %d" % genome.cost_heal_during_threat)
	lines.append("const COST_HEAL_HEALTH_SCALE := %.2f" % genome.cost_heal_health_scale)
	lines.append("const COST_HEAL_COMBAT_ACTIVE := %d" % genome.cost_heal_combat_active)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Shoot")
	lines.append("const COST_SHOOT_LAST_AMMO := %d" % genome.cost_shoot_last_ammo)
	lines.append("const COST_SHOOT_TARGET_IN_COVER := %d" % genome.cost_shoot_target_in_cover)
	lines.append("const COST_SHOOT_HIGH_ACCURACY := %d" % genome.cost_shoot_high_accuracy)
	lines.append("")
	lines.append("# Context-Aware Action Costs - Melee")
	lines.append("const COST_MELEE_VS_RANGED := %d" % genome.cost_melee_vs_ranged)
	lines.append("const COST_MELEE_VS_MELEE := %d" % genome.cost_melee_vs_melee)
	lines.append("const COST_MELEE_LOW_HEALTH := %d" % genome.cost_melee_low_health)
	lines.append("")
	lines.append("# Global Modifiers")
	lines.append("const HEALTH_COST_SCALING := %.2f" % genome.health_cost_scaling)
	lines.append("const AMMO_SCARCITY_THRESHOLD := %.2f" % genome.ammo_scarcity_threshold)
	lines.append("const ENGAGEMENT_WINDOW := %.2f" % genome.engagement_window)
	lines.append("")
	lines.append("# Goal Timing (Hysteresis)")
	lines.append("const GOAL_SWITCH_COOLDOWN := %.2f" % genome.goal_switch_cooldown)
	lines.append("const DEFENSIVE_GOAL_COMMITMENT := %.2f" % genome.defensive_goal_commitment)
	lines.append("const ATTACK_GOAL_COMMITMENT := %.2f" % genome.attack_goal_commitment)
	lines.append("const DEFENSIVE_TIMEOUT := %.2f" % genome.defensive_timeout)
	lines.append("")
	lines.append("# Combat Thresholds")
	lines.append("const RETREAT_DISTANCE := %.1f" % genome.retreat_distance)
	lines.append("const CLOSE_GAP_THRESHOLD := %.1f" % genome.close_gap_threshold)
	lines.append("const TOO_CLOSE_THRESHOLD := %.1f" % genome.too_close_threshold)
	lines.append("const LOW_HEALTH_THRESHOLD := %d" % genome.low_health_threshold)
	lines.append("const HEALTHY_THRESHOLD := %d" % genome.healthy_threshold)
	lines.append("")
	lines.append("# Movement Speeds")
	lines.append("const RANGED_MOVE_SPEED := %.1f" % genome.ranged_move_speed)
	lines.append("const MELEE_MOVE_SPEED := %.1f" % genome.melee_move_speed)
	lines.append("")
	lines.append("# Weapon-Type Multipliers - Ranged")
	lines.append("const RANGED_THREAT_DISTANCE_MULT := %.2f" % genome.ranged_threat_distance_mult)
	lines.append("const RANGED_AMMO_PROXIMITY_MULT := %.2f" % genome.ranged_ammo_proximity_mult)
	lines.append("const RANGED_HEALTH_PROXIMITY_MULT := %.2f" % genome.ranged_health_proximity_mult)
	lines.append("const RANGED_COVER_PROXIMITY_MULT := %.2f" % genome.ranged_cover_proximity_mult)
	lines.append("const RANGED_CENTER_PROXIMITY_MULT := %.2f" % genome.ranged_center_proximity_mult)
	lines.append("const RANGED_STRAFE_PREFERENCE_MULT := %.2f" % genome.ranged_strafe_preference_mult)
	lines.append("const RANGED_SAFE_DISTANCE_MULT := %.2f" % genome.ranged_safe_distance_mult)
	lines.append("const RANGED_DANGER_DISTANCE_MULT := %.2f" % genome.ranged_danger_distance_mult)
	lines.append("")
	lines.append("# Weapon-Type Multipliers - Melee")
	lines.append("const MELEE_THREAT_DISTANCE_MULT := %.2f" % genome.melee_threat_distance_mult)
	lines.append("const MELEE_AMMO_PROXIMITY_MULT := %.2f" % genome.melee_ammo_proximity_mult)
	lines.append("const MELEE_HEALTH_PROXIMITY_MULT := %.2f" % genome.melee_health_proximity_mult)
	lines.append("const MELEE_COVER_PROXIMITY_MULT := %.2f" % genome.melee_cover_proximity_mult)
	lines.append("const MELEE_CENTER_PROXIMITY_MULT := %.2f" % genome.melee_center_proximity_mult)
	lines.append("const MELEE_STRAFE_PREFERENCE_MULT := %.2f" % genome.melee_strafe_preference_mult)
	lines.append("const MELEE_SAFE_DISTANCE_MULT := %.2f" % genome.melee_safe_distance_mult)
	lines.append("const MELEE_DANGER_DISTANCE_MULT := %.2f" % genome.melee_danger_distance_mult)
	lines.append("")
	lines.append("# Position Evaluation - Base Weights")
	lines.append("const POS_WEIGHT_THREAT_DISTANCE := %.2f" % genome.pos_weight_threat_distance)
	lines.append("const POS_WEIGHT_AMMO_PROXIMITY := %.2f" % genome.pos_weight_ammo_proximity)
	lines.append("const POS_WEIGHT_HEALTH_PROXIMITY := %.2f" % genome.pos_weight_health_proximity)
	lines.append("const POS_WEIGHT_COVER_PROXIMITY := %.2f" % genome.pos_weight_cover_proximity)
	lines.append("const POS_WEIGHT_CENTER_PROXIMITY := %.2f" % genome.pos_weight_center_proximity)
	lines.append("const POS_WEIGHT_STRAFE_PREFERENCE := %.2f" % genome.pos_weight_strafe_preference)
	lines.append("const POS_WEIGHT_LOS_TO_TARGET := %.2f" % genome.pos_weight_los_to_target)
	lines.append("const POS_WEIGHT_SPEED_BOOST := %.2f" % genome.pos_weight_speed_boost)
	lines.append("")
	lines.append("# Position Evaluation - Urgency Scaling")
	lines.append("const POS_AMMO_URGENCY_SCALE := %.2f" % genome.pos_ammo_urgency_scale)
	lines.append("const POS_HEALTH_URGENCY_SCALE := %.2f" % genome.pos_health_urgency_scale)
	lines.append("const POS_LOW_AMMO_THRESHOLD := %.2f" % genome.pos_low_ammo_threshold)
	lines.append("const POS_LOW_HEALTH_THRESHOLD := %.2f" % genome.pos_low_health_threshold)
	lines.append("")
	lines.append("# Position Evaluation - Movement Parameters")
	lines.append("const POS_STEP_DISTANCE := %.1f" % genome.pos_step_distance)
	lines.append("const POS_WALL_AVOIDANCE := %.2f" % genome.pos_wall_avoidance)
	lines.append("const POS_SAFE_DISTANCE := %.1f" % genome.pos_safe_distance)
	lines.append("const POS_DANGER_DISTANCE := %.1f" % genome.pos_danger_distance)
	lines.append("")
	lines.append("# Position Evaluation - Mode Multipliers")
	lines.append("const POS_RETREAT_THREAT_MULT := %.2f" % genome.pos_retreat_threat_mult)
	lines.append("const POS_KITE_STRAFE_MULT := %.2f" % genome.pos_kite_strafe_mult)
	lines.append("const POS_APPROACH_SPEED_MULT := %.2f" % genome.pos_approach_speed_mult)

	return "\n".join(lines)


## Creates a genome from the current default values in GOAPConfig
static func create_from_defaults() -> Resource:
	var genome = WeightGenomeClass.new()
	# Genome constructor already uses default values matching GOAPConfig
	return genome
