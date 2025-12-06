## Weight Genome
## Represents a complete set of tunable weights for a GOAP agent.
## Used by the evolutionary optimizer to evolve competitive agent configurations.
##
## Weight Categories:
## - Action base costs: How expensive each action is
## - Dynamic cost modifiers: Context-sensitive cost adjustments
## - Goal timing: Hysteresis and commitment times
## - Combat thresholds: Distance and health triggers
class_name WeightGenome
extends Resource

# ============================================
# ACTION BASE COSTS
# ============================================
## Base costs for each action type (lower = preferred)
@export_group("Action Base Costs")
@export_range(1, 20) var cost_go_to_weapon := 3
@export_range(1, 20) var cost_pickup_weapon := 1
@export_range(1, 20) var cost_go_to_ammo := 3
@export_range(1, 20) var cost_load_weapon := 1
@export_range(1, 20) var cost_go_to_cover := 2
@export_range(1, 20) var cost_take_cover := 1
@export_range(1, 20) var cost_leave_cover := 1
@export_range(1, 20) var cost_approach_target := 4
@export_range(1, 20) var cost_flank := 5
@export_range(1, 20) var cost_shoot := 1
@export_range(1, 20) var cost_melee_attack := 8
@export_range(1, 20) var cost_retreat := 3

# ============================================
# DYNAMIC COST MODIFIERS
# ============================================
## Modifiers applied based on tactical context
@export_group("Dynamic Cost Modifiers")

# Preference bonuses/penalties for weapon pickup
@export_range(-10, 10) var mod_preferred_weapon_bonus := -3  # Bonus for preferred weapon type
@export_range(-10, 10) var mod_nonpreferred_weapon_penalty := 5  # Penalty for non-preferred

# Distance-based modifiers for weapon pickup
@export_range(-10, 10) var mod_close_pickup_bonus := -2  # Bonus when pickup is close
@export_range(-10, 10) var mod_far_pickup_penalty := 3  # Penalty when pickup is far

# Counter-play modifiers
@export_range(-10, 10) var mod_counter_weapon_bonus := -2  # Bonus when countering enemy weapon
@export_range(-10, 10) var mod_same_weapon_penalty := 1  # Penalty for same weapon type matchup

# Tactical situation modifiers
@export_range(-10, 10) var mod_enemy_close_melee_bonus := -3  # Melee bonus when enemy close
@export_range(-10, 10) var mod_enemy_far_melee_penalty := 2  # Melee penalty when enemy far
@export_range(-10, 10) var mod_no_ammo_melee_bonus := -2  # Melee bonus when no ammo available

# Threat-based cost overrides
@export_range(1, 200) var cost_cover_vs_melee := 100  # Cover cost when facing melee (high = avoid)
@export_range(1, 10) var cost_cover_vs_ranged := 1  # Cover cost when facing ranged (low = prefer)
@export_range(1, 10) var cost_retreat_vs_melee := 1  # Retreat cost vs melee (low = prefer)
@export_range(1, 200) var cost_ammo_with_melee := 1000  # Ammo cost when has melee (high = ignore)

# ============================================
# CONTEXT-AWARE ACTION COSTS
# ============================================
## Per-action modifiers based on tactical context (using new blackboard vars)
@export_group("Flank Action")
@export_range(-10, 10) var cost_flank_target_in_cover := -3  # Cheaper when target hiding (flanking useful)
@export_range(-10, 10) var cost_flank_target_exposed := 4  # More expensive when target in open
@export_range(-10, 10) var cost_flank_low_health := 3  # More expensive when low health (risky)

@export_group("Kite Attack")
@export_range(-10, 10) var cost_kite_vs_melee := -4  # Cheaper vs melee (kiting ideal)
@export_range(-10, 10) var cost_kite_low_ammo := 5  # More expensive when ammo scarce
@export_range(-10, 10) var cost_kite_high_health := -2  # Cheaper when healthy (can trade)

@export_group("Close Gap")
@export_range(-10, 10) var cost_close_vs_ranged := 4  # More expensive vs ranged enemy
@export_range(-10, 10) var cost_close_low_health := 5  # More expensive when health critical
@export_range(-10, 10) var cost_close_target_wounded := -3  # Cheaper when target is hurt

@export_group("Approach Target")
@export_range(-10, 10) var cost_approach_target_in_cover := 3  # More expensive vs entrenched enemy
@export_range(-10, 10) var cost_approach_wounded_target := -2  # Cheaper vs wounded target
@export_range(-10, 10) var cost_approach_recent_damage := 4  # More expensive if recently hurt

@export_group("Take Cover")
@export_range(-10, 10) var cost_take_cover_melee_range := 6  # More expensive in melee range (escape first!)
@export_range(-10, 10) var cost_take_cover_exposed := -3  # Cheaper when exposed with ranged threat
@export_range(-10, 10) var cost_take_cover_low_health := -4  # Cheaper when low health

@export_group("Leave Cover")
@export_range(-10, 10) var cost_leave_cover_ranged_threat := 5  # More expensive with ranged threat
@export_range(-10, 10) var cost_leave_cover_melee_threat := -3  # Cheaper to escape melee
@export_range(-10, 10) var cost_leave_cover_high_health := -2  # Cheaper when healthy

@export_group("Wait In Cover")
@export_range(-10, 10) var cost_wait_low_ammo := 4  # More expensive to encourage reloading
@export_range(-10, 10) var cost_wait_low_health := 5  # More expensive to encourage healing
@export_range(-10, 10) var cost_wait_long_combat := 3  # More expensive as combat drags on

@export_group("Heal Action")
@export_range(-10, 10) var cost_heal_during_threat := 4  # More expensive when actively threatened
@export_range(0.0, 2.0) var cost_heal_health_scale := 1.0  # Scale cost by health gap (higher gap = cheaper)
@export_range(-10, 10) var cost_heal_combat_active := 3  # More expensive in active combat

@export_group("Shoot Action")
@export_range(-10, 10) var cost_shoot_last_ammo := 4  # Penalty for using final shot
@export_range(-10, 10) var cost_shoot_target_in_cover := 3  # More expensive when target in cover
@export_range(-10, 10) var cost_shoot_high_accuracy := -2  # Cheaper when target is close/exposed

@export_group("Melee Attack")
@export_range(-10, 10) var cost_melee_vs_ranged := -3  # Bonus when enemy has ranged (counter)
@export_range(-10, 10) var cost_melee_vs_melee := 2  # Penalty for melee vs melee (fair fight)
@export_range(-10, 10) var cost_melee_low_health := 4  # More expensive when low health (risky)

@export_group("Global Modifiers")
@export_range(0.0, 2.0) var health_cost_scaling := 1.0  # How much health affects all action costs
@export_range(0.0, 1.0) var ammo_scarcity_threshold := 0.3  # Below this triggers conservation mode
@export_range(1.0, 5.0) var engagement_window := 2.0  # Seconds of engagement window for timing

# ============================================
# GOAL TIMING (Hysteresis)
# ============================================
## Controls how long agent commits to goals before switching
@export_group("Goal Timing")
@export_range(0.1, 2.0) var goal_switch_cooldown := 0.5  # Min time between any switches
@export_range(0.1, 3.0) var defensive_goal_commitment := 1.0  # Min time in defensive goals
@export_range(0.1, 2.0) var attack_goal_commitment := 0.3  # Min time in attack goal
@export_range(1.0, 10.0) var defensive_timeout := 4.0  # Max time in defense before forcing attack

# ============================================
# COMBAT THRESHOLDS
# ============================================
## Distance and health thresholds that affect behavior
@export_group("Combat Thresholds")
@export_range(50, 300) var retreat_distance := 300.0  # Distance ranged wants to maintain
@export_range(50, 300) var close_gap_threshold := 150.0  # Distance melee wants to close to
@export_range(50, 400) var too_close_threshold := 200.0  # Distance ranged feels threatened
@export_range(10, 80) var low_health_threshold := 50  # Health level to trigger survival mode
@export_range(50, 100) var healthy_threshold := 80  # Health level considered healthy

# ============================================
# MOVE SPEEDS
# ============================================
@export_group("Movement")
@export_range(150, 500) var ranged_move_speed := 350.0
@export_range(150, 500) var melee_move_speed := 280.0

# ============================================
# FITNESS TRACKING (not saved, runtime only)
# ============================================
var fitness := 0.0
var wins := 0
var losses := 0
var damage_dealt := 0.0
var damage_taken := 0.0
var survival_time := 0.0

# Generation tracking
var generation := 0
var parent_ids: Array[int] = []


## Creates a random genome with values within valid ranges
static func create_random() -> Resource:
	var script := load("res://demo/ai/goap/optimization/weight_genome.gd")
	var genome = script.new()

	# Randomize action costs
	genome.cost_go_to_weapon = randi_range(1, 8)
	genome.cost_pickup_weapon = randi_range(1, 4)
	genome.cost_go_to_ammo = randi_range(1, 8)
	genome.cost_load_weapon = randi_range(1, 4)
	genome.cost_go_to_cover = randi_range(1, 6)
	genome.cost_take_cover = randi_range(1, 4)
	genome.cost_leave_cover = randi_range(1, 4)
	genome.cost_approach_target = randi_range(2, 10)
	genome.cost_flank = randi_range(3, 12)
	genome.cost_shoot = randi_range(1, 4)
	genome.cost_melee_attack = randi_range(4, 15)
	genome.cost_retreat = randi_range(1, 8)

	# Randomize modifiers
	genome.mod_preferred_weapon_bonus = randi_range(-6, 0)
	genome.mod_nonpreferred_weapon_penalty = randi_range(0, 8)
	genome.mod_close_pickup_bonus = randi_range(-5, 0)
	genome.mod_far_pickup_penalty = randi_range(0, 6)
	genome.mod_counter_weapon_bonus = randi_range(-5, 0)
	genome.mod_same_weapon_penalty = randi_range(0, 4)
	genome.mod_enemy_close_melee_bonus = randi_range(-6, 0)
	genome.mod_enemy_far_melee_penalty = randi_range(0, 5)
	genome.mod_no_ammo_melee_bonus = randi_range(-5, 0)

	# Randomize threat costs
	genome.cost_cover_vs_melee = randi_range(50, 150)
	genome.cost_cover_vs_ranged = randi_range(1, 5)
	genome.cost_retreat_vs_melee = randi_range(1, 5)

	# Randomize context-aware action costs
	# Flank
	genome.cost_flank_target_in_cover = randi_range(-6, 0)
	genome.cost_flank_target_exposed = randi_range(0, 8)
	genome.cost_flank_low_health = randi_range(0, 6)
	# Kite
	genome.cost_kite_vs_melee = randi_range(-8, 0)
	genome.cost_kite_low_ammo = randi_range(0, 10)
	genome.cost_kite_high_health = randi_range(-5, 0)
	# Close Gap
	genome.cost_close_vs_ranged = randi_range(0, 8)
	genome.cost_close_low_health = randi_range(0, 10)
	genome.cost_close_target_wounded = randi_range(-6, 0)
	# Approach
	genome.cost_approach_target_in_cover = randi_range(0, 6)
	genome.cost_approach_wounded_target = randi_range(-5, 0)
	genome.cost_approach_recent_damage = randi_range(0, 8)
	# Take Cover
	genome.cost_take_cover_melee_range = randi_range(0, 10)
	genome.cost_take_cover_exposed = randi_range(-6, 0)
	genome.cost_take_cover_low_health = randi_range(-8, 0)
	# Leave Cover
	genome.cost_leave_cover_ranged_threat = randi_range(0, 10)
	genome.cost_leave_cover_melee_threat = randi_range(-6, 0)
	genome.cost_leave_cover_high_health = randi_range(-5, 0)
	# Wait In Cover
	genome.cost_wait_low_ammo = randi_range(0, 8)
	genome.cost_wait_low_health = randi_range(0, 10)
	genome.cost_wait_long_combat = randi_range(0, 6)
	# Heal
	genome.cost_heal_during_threat = randi_range(0, 8)
	genome.cost_heal_health_scale = randf_range(0.5, 1.5)
	genome.cost_heal_combat_active = randi_range(0, 6)
	# Shoot
	genome.cost_shoot_last_ammo = randi_range(0, 8)
	genome.cost_shoot_target_in_cover = randi_range(0, 6)
	genome.cost_shoot_high_accuracy = randi_range(-5, 0)
	# Melee
	genome.cost_melee_vs_ranged = randi_range(-6, 0)
	genome.cost_melee_vs_melee = randi_range(0, 5)
	genome.cost_melee_low_health = randi_range(0, 8)
	# Global
	genome.health_cost_scaling = randf_range(0.5, 1.5)
	genome.ammo_scarcity_threshold = randf_range(0.2, 0.5)
	genome.engagement_window = randf_range(1.5, 3.5)

	# Randomize timing
	genome.goal_switch_cooldown = randf_range(0.2, 1.0)
	genome.defensive_goal_commitment = randf_range(0.5, 2.0)
	genome.attack_goal_commitment = randf_range(0.1, 0.8)
	genome.defensive_timeout = randf_range(2.0, 6.0)

	# Randomize thresholds
	genome.retreat_distance = randf_range(200.0, 400.0)
	genome.close_gap_threshold = randf_range(100.0, 250.0)
	genome.too_close_threshold = randf_range(150.0, 300.0)
	genome.low_health_threshold = randi_range(30, 70)
	genome.healthy_threshold = randi_range(60, 95)

	# Randomize speeds
	genome.ranged_move_speed = randf_range(280.0, 420.0)
	genome.melee_move_speed = randf_range(220.0, 350.0)

	return genome


## Creates a child genome by crossing over two parents
static func crossover(parent_a: Resource, parent_b: Resource) -> Resource:
	var script := load("res://demo/ai/goap/optimization/weight_genome.gd")
	var child = script.new()

	# For each property, randomly choose from one parent or the other
	# This is uniform crossover
	child.cost_go_to_weapon = parent_a.cost_go_to_weapon if randf() < 0.5 else parent_b.cost_go_to_weapon
	child.cost_pickup_weapon = parent_a.cost_pickup_weapon if randf() < 0.5 else parent_b.cost_pickup_weapon
	child.cost_go_to_ammo = parent_a.cost_go_to_ammo if randf() < 0.5 else parent_b.cost_go_to_ammo
	child.cost_load_weapon = parent_a.cost_load_weapon if randf() < 0.5 else parent_b.cost_load_weapon
	child.cost_go_to_cover = parent_a.cost_go_to_cover if randf() < 0.5 else parent_b.cost_go_to_cover
	child.cost_take_cover = parent_a.cost_take_cover if randf() < 0.5 else parent_b.cost_take_cover
	child.cost_leave_cover = parent_a.cost_leave_cover if randf() < 0.5 else parent_b.cost_leave_cover
	child.cost_approach_target = parent_a.cost_approach_target if randf() < 0.5 else parent_b.cost_approach_target
	child.cost_flank = parent_a.cost_flank if randf() < 0.5 else parent_b.cost_flank
	child.cost_shoot = parent_a.cost_shoot if randf() < 0.5 else parent_b.cost_shoot
	child.cost_melee_attack = parent_a.cost_melee_attack if randf() < 0.5 else parent_b.cost_melee_attack
	child.cost_retreat = parent_a.cost_retreat if randf() < 0.5 else parent_b.cost_retreat

	child.mod_preferred_weapon_bonus = parent_a.mod_preferred_weapon_bonus if randf() < 0.5 else parent_b.mod_preferred_weapon_bonus
	child.mod_nonpreferred_weapon_penalty = parent_a.mod_nonpreferred_weapon_penalty if randf() < 0.5 else parent_b.mod_nonpreferred_weapon_penalty
	child.mod_close_pickup_bonus = parent_a.mod_close_pickup_bonus if randf() < 0.5 else parent_b.mod_close_pickup_bonus
	child.mod_far_pickup_penalty = parent_a.mod_far_pickup_penalty if randf() < 0.5 else parent_b.mod_far_pickup_penalty
	child.mod_counter_weapon_bonus = parent_a.mod_counter_weapon_bonus if randf() < 0.5 else parent_b.mod_counter_weapon_bonus
	child.mod_same_weapon_penalty = parent_a.mod_same_weapon_penalty if randf() < 0.5 else parent_b.mod_same_weapon_penalty
	child.mod_enemy_close_melee_bonus = parent_a.mod_enemy_close_melee_bonus if randf() < 0.5 else parent_b.mod_enemy_close_melee_bonus
	child.mod_enemy_far_melee_penalty = parent_a.mod_enemy_far_melee_penalty if randf() < 0.5 else parent_b.mod_enemy_far_melee_penalty
	child.mod_no_ammo_melee_bonus = parent_a.mod_no_ammo_melee_bonus if randf() < 0.5 else parent_b.mod_no_ammo_melee_bonus

	child.cost_cover_vs_melee = parent_a.cost_cover_vs_melee if randf() < 0.5 else parent_b.cost_cover_vs_melee
	child.cost_cover_vs_ranged = parent_a.cost_cover_vs_ranged if randf() < 0.5 else parent_b.cost_cover_vs_ranged
	child.cost_retreat_vs_melee = parent_a.cost_retreat_vs_melee if randf() < 0.5 else parent_b.cost_retreat_vs_melee
	child.cost_ammo_with_melee = parent_a.cost_ammo_with_melee if randf() < 0.5 else parent_b.cost_ammo_with_melee

	# Context-aware action costs
	child.cost_flank_target_in_cover = parent_a.cost_flank_target_in_cover if randf() < 0.5 else parent_b.cost_flank_target_in_cover
	child.cost_flank_target_exposed = parent_a.cost_flank_target_exposed if randf() < 0.5 else parent_b.cost_flank_target_exposed
	child.cost_flank_low_health = parent_a.cost_flank_low_health if randf() < 0.5 else parent_b.cost_flank_low_health
	child.cost_kite_vs_melee = parent_a.cost_kite_vs_melee if randf() < 0.5 else parent_b.cost_kite_vs_melee
	child.cost_kite_low_ammo = parent_a.cost_kite_low_ammo if randf() < 0.5 else parent_b.cost_kite_low_ammo
	child.cost_kite_high_health = parent_a.cost_kite_high_health if randf() < 0.5 else parent_b.cost_kite_high_health
	child.cost_close_vs_ranged = parent_a.cost_close_vs_ranged if randf() < 0.5 else parent_b.cost_close_vs_ranged
	child.cost_close_low_health = parent_a.cost_close_low_health if randf() < 0.5 else parent_b.cost_close_low_health
	child.cost_close_target_wounded = parent_a.cost_close_target_wounded if randf() < 0.5 else parent_b.cost_close_target_wounded
	child.cost_approach_target_in_cover = parent_a.cost_approach_target_in_cover if randf() < 0.5 else parent_b.cost_approach_target_in_cover
	child.cost_approach_wounded_target = parent_a.cost_approach_wounded_target if randf() < 0.5 else parent_b.cost_approach_wounded_target
	child.cost_approach_recent_damage = parent_a.cost_approach_recent_damage if randf() < 0.5 else parent_b.cost_approach_recent_damage
	child.cost_take_cover_melee_range = parent_a.cost_take_cover_melee_range if randf() < 0.5 else parent_b.cost_take_cover_melee_range
	child.cost_take_cover_exposed = parent_a.cost_take_cover_exposed if randf() < 0.5 else parent_b.cost_take_cover_exposed
	child.cost_take_cover_low_health = parent_a.cost_take_cover_low_health if randf() < 0.5 else parent_b.cost_take_cover_low_health
	child.cost_leave_cover_ranged_threat = parent_a.cost_leave_cover_ranged_threat if randf() < 0.5 else parent_b.cost_leave_cover_ranged_threat
	child.cost_leave_cover_melee_threat = parent_a.cost_leave_cover_melee_threat if randf() < 0.5 else parent_b.cost_leave_cover_melee_threat
	child.cost_leave_cover_high_health = parent_a.cost_leave_cover_high_health if randf() < 0.5 else parent_b.cost_leave_cover_high_health
	child.cost_wait_low_ammo = parent_a.cost_wait_low_ammo if randf() < 0.5 else parent_b.cost_wait_low_ammo
	child.cost_wait_low_health = parent_a.cost_wait_low_health if randf() < 0.5 else parent_b.cost_wait_low_health
	child.cost_wait_long_combat = parent_a.cost_wait_long_combat if randf() < 0.5 else parent_b.cost_wait_long_combat
	child.cost_heal_during_threat = parent_a.cost_heal_during_threat if randf() < 0.5 else parent_b.cost_heal_during_threat
	child.cost_heal_health_scale = parent_a.cost_heal_health_scale if randf() < 0.5 else parent_b.cost_heal_health_scale
	child.cost_heal_combat_active = parent_a.cost_heal_combat_active if randf() < 0.5 else parent_b.cost_heal_combat_active
	child.cost_shoot_last_ammo = parent_a.cost_shoot_last_ammo if randf() < 0.5 else parent_b.cost_shoot_last_ammo
	child.cost_shoot_target_in_cover = parent_a.cost_shoot_target_in_cover if randf() < 0.5 else parent_b.cost_shoot_target_in_cover
	child.cost_shoot_high_accuracy = parent_a.cost_shoot_high_accuracy if randf() < 0.5 else parent_b.cost_shoot_high_accuracy
	child.cost_melee_vs_ranged = parent_a.cost_melee_vs_ranged if randf() < 0.5 else parent_b.cost_melee_vs_ranged
	child.cost_melee_vs_melee = parent_a.cost_melee_vs_melee if randf() < 0.5 else parent_b.cost_melee_vs_melee
	child.cost_melee_low_health = parent_a.cost_melee_low_health if randf() < 0.5 else parent_b.cost_melee_low_health
	child.health_cost_scaling = parent_a.health_cost_scaling if randf() < 0.5 else parent_b.health_cost_scaling
	child.ammo_scarcity_threshold = parent_a.ammo_scarcity_threshold if randf() < 0.5 else parent_b.ammo_scarcity_threshold
	child.engagement_window = parent_a.engagement_window if randf() < 0.5 else parent_b.engagement_window

	child.goal_switch_cooldown = parent_a.goal_switch_cooldown if randf() < 0.5 else parent_b.goal_switch_cooldown
	child.defensive_goal_commitment = parent_a.defensive_goal_commitment if randf() < 0.5 else parent_b.defensive_goal_commitment
	child.attack_goal_commitment = parent_a.attack_goal_commitment if randf() < 0.5 else parent_b.attack_goal_commitment
	child.defensive_timeout = parent_a.defensive_timeout if randf() < 0.5 else parent_b.defensive_timeout

	child.retreat_distance = parent_a.retreat_distance if randf() < 0.5 else parent_b.retreat_distance
	child.close_gap_threshold = parent_a.close_gap_threshold if randf() < 0.5 else parent_b.close_gap_threshold
	child.too_close_threshold = parent_a.too_close_threshold if randf() < 0.5 else parent_b.too_close_threshold
	child.low_health_threshold = parent_a.low_health_threshold if randf() < 0.5 else parent_b.low_health_threshold
	child.healthy_threshold = parent_a.healthy_threshold if randf() < 0.5 else parent_b.healthy_threshold

	child.ranged_move_speed = parent_a.ranged_move_speed if randf() < 0.5 else parent_b.ranged_move_speed
	child.melee_move_speed = parent_a.melee_move_speed if randf() < 0.5 else parent_b.melee_move_speed

	return child


## Mutates genome in place with given mutation rate
func mutate(mutation_rate: float = 0.1, mutation_strength: float = 0.2) -> void:
	# Action costs
	if randf() < mutation_rate:
		cost_go_to_weapon = clampi(cost_go_to_weapon + randi_range(-2, 2), 1, 20)
	if randf() < mutation_rate:
		cost_pickup_weapon = clampi(cost_pickup_weapon + randi_range(-1, 1), 1, 20)
	if randf() < mutation_rate:
		cost_go_to_ammo = clampi(cost_go_to_ammo + randi_range(-2, 2), 1, 20)
	if randf() < mutation_rate:
		cost_load_weapon = clampi(cost_load_weapon + randi_range(-1, 1), 1, 20)
	if randf() < mutation_rate:
		cost_go_to_cover = clampi(cost_go_to_cover + randi_range(-2, 2), 1, 20)
	if randf() < mutation_rate:
		cost_take_cover = clampi(cost_take_cover + randi_range(-1, 1), 1, 20)
	if randf() < mutation_rate:
		cost_leave_cover = clampi(cost_leave_cover + randi_range(-1, 1), 1, 20)
	if randf() < mutation_rate:
		cost_approach_target = clampi(cost_approach_target + randi_range(-2, 2), 1, 20)
	if randf() < mutation_rate:
		cost_flank = clampi(cost_flank + randi_range(-2, 2), 1, 20)
	if randf() < mutation_rate:
		cost_shoot = clampi(cost_shoot + randi_range(-1, 1), 1, 20)
	if randf() < mutation_rate:
		cost_melee_attack = clampi(cost_melee_attack + randi_range(-3, 3), 1, 20)
	if randf() < mutation_rate:
		cost_retreat = clampi(cost_retreat + randi_range(-2, 2), 1, 20)

	# Modifiers
	if randf() < mutation_rate:
		mod_preferred_weapon_bonus = clampi(mod_preferred_weapon_bonus + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		mod_nonpreferred_weapon_penalty = clampi(mod_nonpreferred_weapon_penalty + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		mod_close_pickup_bonus = clampi(mod_close_pickup_bonus + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		mod_far_pickup_penalty = clampi(mod_far_pickup_penalty + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		mod_counter_weapon_bonus = clampi(mod_counter_weapon_bonus + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		mod_same_weapon_penalty = clampi(mod_same_weapon_penalty + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		mod_enemy_close_melee_bonus = clampi(mod_enemy_close_melee_bonus + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		mod_enemy_far_melee_penalty = clampi(mod_enemy_far_melee_penalty + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		mod_no_ammo_melee_bonus = clampi(mod_no_ammo_melee_bonus + randi_range(-2, 2), -10, 10)

	# Threat costs
	if randf() < mutation_rate:
		cost_cover_vs_melee = clampi(cost_cover_vs_melee + randi_range(-20, 20), 1, 200)
	if randf() < mutation_rate:
		cost_cover_vs_ranged = clampi(cost_cover_vs_ranged + randi_range(-2, 2), 1, 10)
	if randf() < mutation_rate:
		cost_retreat_vs_melee = clampi(cost_retreat_vs_melee + randi_range(-2, 2), 1, 10)

	# Context-aware action costs
	# Flank
	if randf() < mutation_rate:
		cost_flank_target_in_cover = clampi(cost_flank_target_in_cover + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_flank_target_exposed = clampi(cost_flank_target_exposed + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_flank_low_health = clampi(cost_flank_low_health + randi_range(-2, 2), -10, 10)
	# Kite
	if randf() < mutation_rate:
		cost_kite_vs_melee = clampi(cost_kite_vs_melee + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_kite_low_ammo = clampi(cost_kite_low_ammo + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_kite_high_health = clampi(cost_kite_high_health + randi_range(-2, 2), -10, 10)
	# Close Gap
	if randf() < mutation_rate:
		cost_close_vs_ranged = clampi(cost_close_vs_ranged + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_close_low_health = clampi(cost_close_low_health + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_close_target_wounded = clampi(cost_close_target_wounded + randi_range(-2, 2), -10, 10)
	# Approach
	if randf() < mutation_rate:
		cost_approach_target_in_cover = clampi(cost_approach_target_in_cover + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_approach_wounded_target = clampi(cost_approach_wounded_target + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_approach_recent_damage = clampi(cost_approach_recent_damage + randi_range(-2, 2), -10, 10)
	# Take Cover
	if randf() < mutation_rate:
		cost_take_cover_melee_range = clampi(cost_take_cover_melee_range + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_take_cover_exposed = clampi(cost_take_cover_exposed + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_take_cover_low_health = clampi(cost_take_cover_low_health + randi_range(-2, 2), -10, 10)
	# Leave Cover
	if randf() < mutation_rate:
		cost_leave_cover_ranged_threat = clampi(cost_leave_cover_ranged_threat + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_leave_cover_melee_threat = clampi(cost_leave_cover_melee_threat + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_leave_cover_high_health = clampi(cost_leave_cover_high_health + randi_range(-2, 2), -10, 10)
	# Wait In Cover
	if randf() < mutation_rate:
		cost_wait_low_ammo = clampi(cost_wait_low_ammo + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_wait_low_health = clampi(cost_wait_low_health + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_wait_long_combat = clampi(cost_wait_long_combat + randi_range(-2, 2), -10, 10)
	# Heal
	if randf() < mutation_rate:
		cost_heal_during_threat = clampi(cost_heal_during_threat + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_heal_health_scale = clampf(cost_heal_health_scale + randf_range(-0.2, 0.2), 0.0, 2.0)
	if randf() < mutation_rate:
		cost_heal_combat_active = clampi(cost_heal_combat_active + randi_range(-2, 2), -10, 10)
	# Shoot
	if randf() < mutation_rate:
		cost_shoot_last_ammo = clampi(cost_shoot_last_ammo + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_shoot_target_in_cover = clampi(cost_shoot_target_in_cover + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_shoot_high_accuracy = clampi(cost_shoot_high_accuracy + randi_range(-2, 2), -10, 10)
	# Melee
	if randf() < mutation_rate:
		cost_melee_vs_ranged = clampi(cost_melee_vs_ranged + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_melee_vs_melee = clampi(cost_melee_vs_melee + randi_range(-2, 2), -10, 10)
	if randf() < mutation_rate:
		cost_melee_low_health = clampi(cost_melee_low_health + randi_range(-2, 2), -10, 10)
	# Global
	if randf() < mutation_rate:
		health_cost_scaling = clampf(health_cost_scaling + randf_range(-0.2, 0.2), 0.0, 2.0)
	if randf() < mutation_rate:
		ammo_scarcity_threshold = clampf(ammo_scarcity_threshold + randf_range(-0.1, 0.1), 0.0, 1.0)
	if randf() < mutation_rate:
		engagement_window = clampf(engagement_window + randf_range(-0.5, 0.5), 1.0, 5.0)

	# Timing (use float mutations)
	if randf() < mutation_rate:
		goal_switch_cooldown = clampf(goal_switch_cooldown + randf_range(-0.2, 0.2), 0.1, 2.0)
	if randf() < mutation_rate:
		defensive_goal_commitment = clampf(defensive_goal_commitment + randf_range(-0.3, 0.3), 0.1, 3.0)
	if randf() < mutation_rate:
		attack_goal_commitment = clampf(attack_goal_commitment + randf_range(-0.2, 0.2), 0.1, 2.0)
	if randf() < mutation_rate:
		defensive_timeout = clampf(defensive_timeout + randf_range(-1.0, 1.0), 1.0, 10.0)

	# Thresholds
	if randf() < mutation_rate:
		retreat_distance = clampf(retreat_distance + randf_range(-50.0, 50.0), 50.0, 500.0)
	if randf() < mutation_rate:
		close_gap_threshold = clampf(close_gap_threshold + randf_range(-30.0, 30.0), 50.0, 300.0)
	if randf() < mutation_rate:
		too_close_threshold = clampf(too_close_threshold + randf_range(-40.0, 40.0), 50.0, 400.0)
	if randf() < mutation_rate:
		low_health_threshold = clampi(low_health_threshold + randi_range(-10, 10), 10, 80)
	if randf() < mutation_rate:
		healthy_threshold = clampi(healthy_threshold + randi_range(-10, 10), 50, 100)

	# Speeds
	if randf() < mutation_rate:
		ranged_move_speed = clampf(ranged_move_speed + randf_range(-40.0, 40.0), 150.0, 500.0)
	if randf() < mutation_rate:
		melee_move_speed = clampf(melee_move_speed + randf_range(-40.0, 40.0), 150.0, 500.0)


## Resets fitness tracking for a new evaluation round
func reset_fitness() -> void:
	fitness = 0.0
	wins = 0
	losses = 0
	damage_dealt = 0.0
	damage_taken = 0.0
	survival_time = 0.0


## Calculates fitness score based on battle performance
func calculate_fitness() -> float:
	# Multi-objective fitness:
	# - Wins are most important
	# - Damage efficiency matters (deal more than you take)
	# - Survival time as a tiebreaker
	var win_score := wins * 100.0
	var damage_score := (damage_dealt - damage_taken) * 0.5
	var survival_score := survival_time * 0.1

	fitness = win_score + damage_score + survival_score
	return fitness


## Creates a deep copy of this genome
func duplicate_genome() -> Resource:
	var script := load("res://demo/ai/goap/optimization/weight_genome.gd")
	var copy = script.new()

	copy.cost_go_to_weapon = cost_go_to_weapon
	copy.cost_pickup_weapon = cost_pickup_weapon
	copy.cost_go_to_ammo = cost_go_to_ammo
	copy.cost_load_weapon = cost_load_weapon
	copy.cost_go_to_cover = cost_go_to_cover
	copy.cost_take_cover = cost_take_cover
	copy.cost_leave_cover = cost_leave_cover
	copy.cost_approach_target = cost_approach_target
	copy.cost_flank = cost_flank
	copy.cost_shoot = cost_shoot
	copy.cost_melee_attack = cost_melee_attack
	copy.cost_retreat = cost_retreat

	copy.mod_preferred_weapon_bonus = mod_preferred_weapon_bonus
	copy.mod_nonpreferred_weapon_penalty = mod_nonpreferred_weapon_penalty
	copy.mod_close_pickup_bonus = mod_close_pickup_bonus
	copy.mod_far_pickup_penalty = mod_far_pickup_penalty
	copy.mod_counter_weapon_bonus = mod_counter_weapon_bonus
	copy.mod_same_weapon_penalty = mod_same_weapon_penalty
	copy.mod_enemy_close_melee_bonus = mod_enemy_close_melee_bonus
	copy.mod_enemy_far_melee_penalty = mod_enemy_far_melee_penalty
	copy.mod_no_ammo_melee_bonus = mod_no_ammo_melee_bonus

	copy.cost_cover_vs_melee = cost_cover_vs_melee
	copy.cost_cover_vs_ranged = cost_cover_vs_ranged
	copy.cost_retreat_vs_melee = cost_retreat_vs_melee
	copy.cost_ammo_with_melee = cost_ammo_with_melee

	# Context-aware action costs
	copy.cost_flank_target_in_cover = cost_flank_target_in_cover
	copy.cost_flank_target_exposed = cost_flank_target_exposed
	copy.cost_flank_low_health = cost_flank_low_health
	copy.cost_kite_vs_melee = cost_kite_vs_melee
	copy.cost_kite_low_ammo = cost_kite_low_ammo
	copy.cost_kite_high_health = cost_kite_high_health
	copy.cost_close_vs_ranged = cost_close_vs_ranged
	copy.cost_close_low_health = cost_close_low_health
	copy.cost_close_target_wounded = cost_close_target_wounded
	copy.cost_approach_target_in_cover = cost_approach_target_in_cover
	copy.cost_approach_wounded_target = cost_approach_wounded_target
	copy.cost_approach_recent_damage = cost_approach_recent_damage
	copy.cost_take_cover_melee_range = cost_take_cover_melee_range
	copy.cost_take_cover_exposed = cost_take_cover_exposed
	copy.cost_take_cover_low_health = cost_take_cover_low_health
	copy.cost_leave_cover_ranged_threat = cost_leave_cover_ranged_threat
	copy.cost_leave_cover_melee_threat = cost_leave_cover_melee_threat
	copy.cost_leave_cover_high_health = cost_leave_cover_high_health
	copy.cost_wait_low_ammo = cost_wait_low_ammo
	copy.cost_wait_low_health = cost_wait_low_health
	copy.cost_wait_long_combat = cost_wait_long_combat
	copy.cost_heal_during_threat = cost_heal_during_threat
	copy.cost_heal_health_scale = cost_heal_health_scale
	copy.cost_heal_combat_active = cost_heal_combat_active
	copy.cost_shoot_last_ammo = cost_shoot_last_ammo
	copy.cost_shoot_target_in_cover = cost_shoot_target_in_cover
	copy.cost_shoot_high_accuracy = cost_shoot_high_accuracy
	copy.cost_melee_vs_ranged = cost_melee_vs_ranged
	copy.cost_melee_vs_melee = cost_melee_vs_melee
	copy.cost_melee_low_health = cost_melee_low_health
	copy.health_cost_scaling = health_cost_scaling
	copy.ammo_scarcity_threshold = ammo_scarcity_threshold
	copy.engagement_window = engagement_window

	copy.goal_switch_cooldown = goal_switch_cooldown
	copy.defensive_goal_commitment = defensive_goal_commitment
	copy.attack_goal_commitment = attack_goal_commitment
	copy.defensive_timeout = defensive_timeout

	copy.retreat_distance = retreat_distance
	copy.close_gap_threshold = close_gap_threshold
	copy.too_close_threshold = too_close_threshold
	copy.low_health_threshold = low_health_threshold
	copy.healthy_threshold = healthy_threshold

	copy.ranged_move_speed = ranged_move_speed
	copy.melee_move_speed = melee_move_speed

	copy.generation = generation
	copy.parent_ids = parent_ids.duplicate()

	return copy


## Converts genome to a dictionary for serialization
func to_dict() -> Dictionary:
	return {
		"action_costs": {
			"go_to_weapon": cost_go_to_weapon,
			"pickup_weapon": cost_pickup_weapon,
			"go_to_ammo": cost_go_to_ammo,
			"load_weapon": cost_load_weapon,
			"go_to_cover": cost_go_to_cover,
			"take_cover": cost_take_cover,
			"leave_cover": cost_leave_cover,
			"approach_target": cost_approach_target,
			"flank": cost_flank,
			"shoot": cost_shoot,
			"melee_attack": cost_melee_attack,
			"retreat": cost_retreat,
		},
		"modifiers": {
			"preferred_weapon_bonus": mod_preferred_weapon_bonus,
			"nonpreferred_weapon_penalty": mod_nonpreferred_weapon_penalty,
			"close_pickup_bonus": mod_close_pickup_bonus,
			"far_pickup_penalty": mod_far_pickup_penalty,
			"counter_weapon_bonus": mod_counter_weapon_bonus,
			"same_weapon_penalty": mod_same_weapon_penalty,
			"enemy_close_melee_bonus": mod_enemy_close_melee_bonus,
			"enemy_far_melee_penalty": mod_enemy_far_melee_penalty,
			"no_ammo_melee_bonus": mod_no_ammo_melee_bonus,
		},
		"threat_costs": {
			"cover_vs_melee": cost_cover_vs_melee,
			"cover_vs_ranged": cost_cover_vs_ranged,
			"retreat_vs_melee": cost_retreat_vs_melee,
			"ammo_with_melee": cost_ammo_with_melee,
		},
		"context_costs": {
			"flank_target_in_cover": cost_flank_target_in_cover,
			"flank_target_exposed": cost_flank_target_exposed,
			"flank_low_health": cost_flank_low_health,
			"kite_vs_melee": cost_kite_vs_melee,
			"kite_low_ammo": cost_kite_low_ammo,
			"kite_high_health": cost_kite_high_health,
			"close_vs_ranged": cost_close_vs_ranged,
			"close_low_health": cost_close_low_health,
			"close_target_wounded": cost_close_target_wounded,
			"approach_target_in_cover": cost_approach_target_in_cover,
			"approach_wounded_target": cost_approach_wounded_target,
			"approach_recent_damage": cost_approach_recent_damage,
			"take_cover_melee_range": cost_take_cover_melee_range,
			"take_cover_exposed": cost_take_cover_exposed,
			"take_cover_low_health": cost_take_cover_low_health,
			"leave_cover_ranged_threat": cost_leave_cover_ranged_threat,
			"leave_cover_melee_threat": cost_leave_cover_melee_threat,
			"leave_cover_high_health": cost_leave_cover_high_health,
			"wait_low_ammo": cost_wait_low_ammo,
			"wait_low_health": cost_wait_low_health,
			"wait_long_combat": cost_wait_long_combat,
			"heal_during_threat": cost_heal_during_threat,
			"heal_health_scale": cost_heal_health_scale,
			"heal_combat_active": cost_heal_combat_active,
			"shoot_last_ammo": cost_shoot_last_ammo,
			"shoot_target_in_cover": cost_shoot_target_in_cover,
			"shoot_high_accuracy": cost_shoot_high_accuracy,
			"melee_vs_ranged": cost_melee_vs_ranged,
			"melee_vs_melee": cost_melee_vs_melee,
			"melee_low_health": cost_melee_low_health,
		},
		"global": {
			"health_cost_scaling": health_cost_scaling,
			"ammo_scarcity_threshold": ammo_scarcity_threshold,
			"engagement_window": engagement_window,
		},
		"timing": {
			"goal_switch_cooldown": goal_switch_cooldown,
			"defensive_goal_commitment": defensive_goal_commitment,
			"attack_goal_commitment": attack_goal_commitment,
			"defensive_timeout": defensive_timeout,
		},
		"thresholds": {
			"retreat_distance": retreat_distance,
			"close_gap_threshold": close_gap_threshold,
			"too_close_threshold": too_close_threshold,
			"low_health_threshold": low_health_threshold,
			"healthy_threshold": healthy_threshold,
		},
		"movement": {
			"ranged_move_speed": ranged_move_speed,
			"melee_move_speed": melee_move_speed,
		},
		"meta": {
			"generation": generation,
			"fitness": fitness,
			"wins": wins,
			"losses": losses,
		}
	}


## Loads genome values from a dictionary
static func from_dict(data: Dictionary) -> Resource:
	var script := load("res://demo/ai/goap/optimization/weight_genome.gd")
	var genome = script.new()

	if "action_costs" in data:
		var costs: Dictionary = data["action_costs"]
		genome.cost_go_to_weapon = costs.get("go_to_weapon", 3)
		genome.cost_pickup_weapon = costs.get("pickup_weapon", 1)
		genome.cost_go_to_ammo = costs.get("go_to_ammo", 3)
		genome.cost_load_weapon = costs.get("load_weapon", 1)
		genome.cost_go_to_cover = costs.get("go_to_cover", 2)
		genome.cost_take_cover = costs.get("take_cover", 1)
		genome.cost_leave_cover = costs.get("leave_cover", 1)
		genome.cost_approach_target = costs.get("approach_target", 4)
		genome.cost_flank = costs.get("flank", 5)
		genome.cost_shoot = costs.get("shoot", 1)
		genome.cost_melee_attack = costs.get("melee_attack", 8)
		genome.cost_retreat = costs.get("retreat", 3)

	if "modifiers" in data:
		var mods: Dictionary = data["modifiers"]
		genome.mod_preferred_weapon_bonus = mods.get("preferred_weapon_bonus", -3)
		genome.mod_nonpreferred_weapon_penalty = mods.get("nonpreferred_weapon_penalty", 5)
		genome.mod_close_pickup_bonus = mods.get("close_pickup_bonus", -2)
		genome.mod_far_pickup_penalty = mods.get("far_pickup_penalty", 3)
		genome.mod_counter_weapon_bonus = mods.get("counter_weapon_bonus", -2)
		genome.mod_same_weapon_penalty = mods.get("same_weapon_penalty", 1)
		genome.mod_enemy_close_melee_bonus = mods.get("enemy_close_melee_bonus", -3)
		genome.mod_enemy_far_melee_penalty = mods.get("enemy_far_melee_penalty", 2)
		genome.mod_no_ammo_melee_bonus = mods.get("no_ammo_melee_bonus", -2)

	if "threat_costs" in data:
		var threats: Dictionary = data["threat_costs"]
		genome.cost_cover_vs_melee = threats.get("cover_vs_melee", 100)
		genome.cost_cover_vs_ranged = threats.get("cover_vs_ranged", 1)
		genome.cost_retreat_vs_melee = threats.get("retreat_vs_melee", 1)
		genome.cost_ammo_with_melee = threats.get("ammo_with_melee", 1000)

	if "context_costs" in data:
		var ctx: Dictionary = data["context_costs"]
		genome.cost_flank_target_in_cover = ctx.get("flank_target_in_cover", -3)
		genome.cost_flank_target_exposed = ctx.get("flank_target_exposed", 4)
		genome.cost_flank_low_health = ctx.get("flank_low_health", 3)
		genome.cost_kite_vs_melee = ctx.get("kite_vs_melee", -4)
		genome.cost_kite_low_ammo = ctx.get("kite_low_ammo", 5)
		genome.cost_kite_high_health = ctx.get("kite_high_health", -2)
		genome.cost_close_vs_ranged = ctx.get("close_vs_ranged", 4)
		genome.cost_close_low_health = ctx.get("close_low_health", 5)
		genome.cost_close_target_wounded = ctx.get("close_target_wounded", -3)
		genome.cost_approach_target_in_cover = ctx.get("approach_target_in_cover", 3)
		genome.cost_approach_wounded_target = ctx.get("approach_wounded_target", -2)
		genome.cost_approach_recent_damage = ctx.get("approach_recent_damage", 4)
		genome.cost_take_cover_melee_range = ctx.get("take_cover_melee_range", 6)
		genome.cost_take_cover_exposed = ctx.get("take_cover_exposed", -3)
		genome.cost_take_cover_low_health = ctx.get("take_cover_low_health", -4)
		genome.cost_leave_cover_ranged_threat = ctx.get("leave_cover_ranged_threat", 5)
		genome.cost_leave_cover_melee_threat = ctx.get("leave_cover_melee_threat", -3)
		genome.cost_leave_cover_high_health = ctx.get("leave_cover_high_health", -2)
		genome.cost_wait_low_ammo = ctx.get("wait_low_ammo", 4)
		genome.cost_wait_low_health = ctx.get("wait_low_health", 5)
		genome.cost_wait_long_combat = ctx.get("wait_long_combat", 3)
		genome.cost_heal_during_threat = ctx.get("heal_during_threat", 4)
		genome.cost_heal_health_scale = ctx.get("heal_health_scale", 1.0)
		genome.cost_heal_combat_active = ctx.get("heal_combat_active", 3)
		genome.cost_shoot_last_ammo = ctx.get("shoot_last_ammo", 4)
		genome.cost_shoot_target_in_cover = ctx.get("shoot_target_in_cover", 3)
		genome.cost_shoot_high_accuracy = ctx.get("shoot_high_accuracy", -2)
		genome.cost_melee_vs_ranged = ctx.get("melee_vs_ranged", -3)
		genome.cost_melee_vs_melee = ctx.get("melee_vs_melee", 2)
		genome.cost_melee_low_health = ctx.get("melee_low_health", 4)

	if "global" in data:
		var glob: Dictionary = data["global"]
		genome.health_cost_scaling = glob.get("health_cost_scaling", 1.0)
		genome.ammo_scarcity_threshold = glob.get("ammo_scarcity_threshold", 0.3)
		genome.engagement_window = glob.get("engagement_window", 2.0)

	if "timing" in data:
		var timing: Dictionary = data["timing"]
		genome.goal_switch_cooldown = timing.get("goal_switch_cooldown", 0.5)
		genome.defensive_goal_commitment = timing.get("defensive_goal_commitment", 1.0)
		genome.attack_goal_commitment = timing.get("attack_goal_commitment", 0.3)
		genome.defensive_timeout = timing.get("defensive_timeout", 4.0)

	if "thresholds" in data:
		var thresh: Dictionary = data["thresholds"]
		genome.retreat_distance = thresh.get("retreat_distance", 300.0)
		genome.close_gap_threshold = thresh.get("close_gap_threshold", 150.0)
		genome.too_close_threshold = thresh.get("too_close_threshold", 200.0)
		genome.low_health_threshold = thresh.get("low_health_threshold", 50)
		genome.healthy_threshold = thresh.get("healthy_threshold", 80)

	if "movement" in data:
		var movement: Dictionary = data["movement"]
		genome.ranged_move_speed = movement.get("ranged_move_speed", 350.0)
		genome.melee_move_speed = movement.get("melee_move_speed", 280.0)

	if "meta" in data:
		var meta: Dictionary = data["meta"]
		genome.generation = meta.get("generation", 0)
		genome.fitness = meta.get("fitness", 0.0)
		genome.wins = meta.get("wins", 0)
		genome.losses = meta.get("losses", 0)

	return genome
