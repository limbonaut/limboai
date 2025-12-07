## GOAP Demo Configuration
## Centralized constants for the GOAP tactical demo
class_name GOAPConfig
extends RefCounted

# Combat ranges (scaled down by 0.6 to make arena feel larger)
const ATTACK_RANGE := 60.0        # Melee attack range (reduced for closer combat)
const SHOOTING_RANGE := 480.0     # Ranged weapon range (was 800)
const PICKUP_RANGE := 50.0        # Distance to collect pickups (was 80)
const COVER_RANGE := 60.0         # Distance to be considered "at cover" (was 100)

# Arena bounds (keeps agents inside playable area - extended for more maneuvering room)
const ARENA_MIN := Vector2(50.0, 50.0)
const ARENA_MAX := Vector2(1870.0, 1030.0)

# Edge awareness thresholds (scaled down by 0.6)
const EDGE_WARNING_DISTANCE := 90.0    # Distance from edge to start considering repositioning (was 150)
const EDGE_DANGER_DISTANCE := 50.0     # Distance at which agent is cornered/trapped (was 80)

# Movement - weapon-specific speeds (OPTIMIZED through evolution - Gen 30)
const MOVE_SPEED := 180.0         # Default base speed
const RANGED_MOVE_SPEED := 426.5  # Optimized: faster kiting for effective retreat
const MELEE_MOVE_SPEED := 371.2   # Optimized: faster gap closing

# ============================================
# WEAPON-TYPE MULTIPLIERS (OPTIMIZED - Gen 30)
# ============================================
# These multipliers adjust position evaluation weights based on weapon type.
# Final weight = base_weight * multiplier for the equipped weapon type.
# Evolved through 30 generations of battle simulation!

# Ranged weapon multipliers (prioritize distance and cover)
const RANGED_THREAT_DISTANCE_MULT := 3.0      # BOOSTED: ranged REALLY wants distance from melee!
const RANGED_AMMO_PROXIMITY_MULT := 2.42      # Optimized: HIGH priority for ammo!
const RANGED_HEALTH_PROXIMITY_MULT := 1.50    # Optimized: moderate health priority
const RANGED_COVER_PROXIMITY_MULT := 2.16     # Optimized: strong cover preference
const RANGED_CENTER_PROXIMITY_MULT := 1.5     # BOOSTED: prefer center for escape routes
const RANGED_STRAFE_PREFERENCE_MULT := 1.5    # BOOSTED: strafe more to evade
const RANGED_SAFE_DISTANCE_MULT := 2.5        # BOOSTED: much larger safe zone
const RANGED_DANGER_DISTANCE_MULT := 2.5      # BOOSTED: react to danger much earlier

# Melee weapon multipliers (prioritize closing distance)
const MELEE_THREAT_DISTANCE_MULT := -0.71     # Optimized: NEGATIVE = approach threats!
const MELEE_AMMO_PROXIMITY_MULT := 0.44       # Optimized: low ammo priority
const MELEE_HEALTH_PROXIMITY_MULT := 1.02     # Optimized: similar health priority
const MELEE_COVER_PROXIMITY_MULT := 0.17      # Optimized: very low cover priority
const MELEE_CENTER_PROXIMITY_MULT := 0.54     # Optimized: reduced center preference
const MELEE_STRAFE_PREFERENCE_MULT := 0.18    # Optimized: direct approach, no strafe
const MELEE_SAFE_DISTANCE_MULT := 0.34        # Optimized: tiny safe zone (aggressive)
const MELEE_DANGER_DISTANCE_MULT := 0.70      # Optimized: close danger zone

# Melee attack balance
const MELEE_SWING_COOLDOWN := 0.8     # Time between melee swings
const MELEE_SWING_SLOW_FACTOR := 0.3  # Speed multiplier during swing (30% speed)

# Health thresholds (OPTIMIZED - Gen 30)
const LOW_HEALTH_THRESHOLD := 45   # Optimized: balanced aggression
const HEALTHY_THRESHOLD := 99      # Optimized: high bar for "healthy"

# Resource defaults
const DEFAULT_MAX_AMMO := 10
const DEFAULT_MAX_HEALTH := 100
const DEFAULT_JAM_CHANCE := 0.0  # Disabled

# Scarcity mode settings
const SCARCITY_MAX_AMMO := 3
const SCARCITY_AMMO_RESPAWN := 12.0
const SCARCITY_HEALTH_RESPAWN := 15.0
const NORMAL_AMMO_RESPAWN := 5.0
const NORMAL_HEALTH_RESPAWN := 8.0

# Physics layers
const LOS_COLLISION_LAYER := 16

# Weapon type combat distances (TUNED for better ranged kiting)
const RETREAT_DISTANCE := 450.0        # INCREASED: ranged needs much more kiting distance
const CLOSE_GAP_THRESHOLD := 214.8     # Melee engages from further away
const TOO_CLOSE_THRESHOLD := 250.0     # INCREASED: ranged retreats earlier
const TOO_FAR_THRESHOLD := 90.0        # Melee needs to close if farther than this

# Suppression
const SUPPRESSION_DURATION := 1.5      # Duration in seconds

# Speed Boost
const SPEED_BOOST_DURATION := 3.5      # Duration in seconds
const SPEED_BOOST_MULTIPLIER := 1.5    # 50% faster movement

# Accuracy
const COVER_ACCURACY_PENALTY := 0.5    # 50% accuracy when shooting from cover

# ============================================
# OPTIMIZED GOAP WEIGHTS
# These values were discovered through evolutionary optimization.
# Run optimization_runner.tscn to discover better values for your game!
# ============================================

# Action Base Costs (OPTIMIZED - Gen 30 - lower = preferred by planner)
const COST_GO_TO_WEAPON := 2       # Optimized: quick weapon pickup
const COST_PICKUP_WEAPON := 4      # Optimized
const COST_GO_TO_AMMO := 5         # Optimized
const COST_LOAD_WEAPON := 1        # Optimized
const COST_GO_TO_COVER := 5        # Optimized
const COST_TAKE_COVER := 2         # Optimized
const COST_LEAVE_COVER := 1        # Optimized: easy to leave cover
const COST_APPROACH_TARGET := 1    # Optimized: very aggressive approach!
const COST_FLANK := 10             # Optimized: flank is expensive
const COST_SHOOT := 2              # Optimized
const COST_MELEE_ATTACK := 7       # Optimized: moderate melee preference
const COST_RETREAT := 8            # Optimized: retreat is expensive - commit to fights

# Dynamic Cost Modifiers (OPTIMIZED - Gen 30)
const MOD_PREFERRED_WEAPON_BONUS := -1     # Optimized: slight preference
const MOD_NONPREFERRED_WEAPON_PENALTY := 0 # Optimized: neutral
const MOD_CLOSE_PICKUP_BONUS := -3         # Optimized: grab nearby pickups
const MOD_FAR_PICKUP_PENALTY := 1          # Optimized: small penalty for far pickups
const MOD_COUNTER_WEAPON_BONUS := 1        # Optimized
const MOD_SAME_WEAPON_PENALTY := -1        # Optimized: slight bonus for same weapon
const MOD_ENEMY_CLOSE_MELEE_BONUS := -1    # Optimized
const MOD_ENEMY_FAR_MELEE_PENALTY := 2     # Optimized
const MOD_NO_AMMO_MELEE_BONUS := -5        # Optimized: strongly prefer melee when out of ammo

# Threat-Based Cost Overrides (OPTIMIZED - Gen 30)
const COST_COVER_VS_MELEE := 139           # Optimized: cover less useful vs melee
const COST_COVER_VS_RANGED := 10           # Optimized: cover good vs ranged
const COST_RETREAT_VS_MELEE := 2           # Optimized
const COST_AMMO_WITH_MELEE := 1000         # Unchanged: avoid ammo when melee threat close

# Goal Timing (Hysteresis) - OPTIMIZED - Gen 30
const GOAL_SWITCH_COOLDOWN := 0.83         # Optimized: longer cooldown between switches
const DEFENSIVE_GOAL_COMMITMENT := 0.40    # Optimized: moderate defensive commitment
const ATTACK_GOAL_COMMITMENT := 0.35       # Optimized: moderate attack commitment
const DEFENSIVE_TIMEOUT := 6.16            # Optimized: longer defensive timeout

# ============================================
# CONTEXT-AWARE ACTION COSTS
# These modifiers adjust action costs based on tactical context
# ============================================

# Flank Action (OPTIMIZED - Gen 30)
const COST_FLANK_TARGET_IN_COVER := 2      # Optimized
const COST_FLANK_TARGET_EXPOSED := 4       # Optimized
const COST_FLANK_LOW_HEALTH := 6           # Optimized

# Kite Attack (OPTIMIZED - Gen 30)
const COST_KITE_VS_MELEE := -5             # Optimized: strongly encourage kiting vs melee!
const COST_KITE_LOW_AMMO := 6              # Optimized
const COST_KITE_HIGH_HEALTH := -7          # Optimized: kite aggressively when healthy

# Close Gap (OPTIMIZED - Gen 30)
const COST_CLOSE_VS_RANGED := -1           # Optimized: close gap vs ranged
const COST_CLOSE_LOW_HEALTH := 2           # Optimized
const COST_CLOSE_TARGET_WOUNDED := 0       # Optimized

# Approach Target (OPTIMIZED - Gen 30)
const COST_APPROACH_TARGET_IN_COVER := 6   # Optimized
const COST_APPROACH_WOUNDED_TARGET := -3   # Optimized: press wounded targets
const COST_APPROACH_RECENT_DAMAGE := 10    # Optimized: caution after taking damage

# Take Cover (OPTIMIZED - Gen 30)
const COST_TAKE_COVER_MELEE_RANGE := 1     # Optimized
const COST_TAKE_COVER_EXPOSED := -2        # Optimized
const COST_TAKE_COVER_LOW_HEALTH := -5     # Optimized: take cover when hurt

# Leave Cover (OPTIMIZED - Gen 30)
const COST_LEAVE_COVER_RANGED_THREAT := 9  # Optimized: stay in cover vs ranged!
const COST_LEAVE_COVER_MELEE_THREAT := 1   # Optimized
const COST_LEAVE_COVER_HIGH_HEALTH := -3   # Optimized

# Wait In Cover (OPTIMIZED - Gen 30)
const COST_WAIT_LOW_AMMO := 8              # Optimized: don't wait when low ammo
const COST_WAIT_LOW_HEALTH := 3            # Optimized
const COST_WAIT_LONG_COMBAT := 3           # Optimized

# Heal Action (OPTIMIZED - Gen 30)
const COST_HEAL_DURING_THREAT := 10        # Optimized: very risky to heal under threat
const COST_HEAL_HEALTH_SCALE := 1.61       # Optimized
const COST_HEAL_COMBAT_ACTIVE := 6         # Optimized

# Shoot Action (OPTIMIZED - Gen 30)
const COST_SHOOT_LAST_AMMO := 3            # Optimized
const COST_SHOOT_TARGET_IN_COVER := 8      # Optimized: avoid shooting targets in cover
const COST_SHOOT_HIGH_ACCURACY := -2       # Optimized: prefer high accuracy shots

# Melee Attack (OPTIMIZED - Gen 30)
const COST_MELEE_VS_RANGED := -1           # Optimized
const COST_MELEE_VS_MELEE := 0             # Optimized
const COST_MELEE_LOW_HEALTH := 4           # Optimized: caution when low health

# Global Modifiers (OPTIMIZED - Gen 30)
const HEALTH_COST_SCALING := 1.38          # Optimized
const AMMO_SCARCITY_THRESHOLD := 0.35      # Optimized
const ENGAGEMENT_WINDOW := 1.56            # Optimized

# ============================================
# POSITION EVALUATION WEIGHTS (OPTIMIZED - Gen 30)
# Used by PositionEvaluator for smart movement decisions
# ============================================

# Base Weights - how much each factor matters
const POS_WEIGHT_THREAT_DISTANCE := 0.58   # Optimized: moderate threat priority
const POS_WEIGHT_AMMO_PROXIMITY := 0.30    # Optimized: lower base ammo priority
const POS_WEIGHT_HEALTH_PROXIMITY := 0.50  # Optimized: moderate health priority
const POS_WEIGHT_COVER_PROXIMITY := 0.65   # Optimized: strong cover preference
const POS_WEIGHT_CENTER_PROXIMITY := 0.35  # Optimized: prefer center
const POS_WEIGHT_STRAFE_PREFERENCE := 0.45 # Optimized: moderate strafe
const POS_WEIGHT_LOS_TO_TARGET := 0.36     # Optimized
const POS_WEIGHT_SPEED_BOOST := 0.63       # Optimized: value speed boosts

# Urgency Scaling - multiply weights when resources low
const POS_AMMO_URGENCY_SCALE := 1.77       # Optimized: strong ammo urgency boost
const POS_HEALTH_URGENCY_SCALE := 2.98     # Optimized: very strong health urgency!
const POS_LOW_AMMO_THRESHOLD := 0.35       # Optimized
const POS_LOW_HEALTH_THRESHOLD := 0.53     # Optimized: trigger urgency earlier

# Movement Parameters
const POS_STEP_DISTANCE := 80.7            # Optimized: smaller movement steps
const POS_WALL_AVOIDANCE := 2.0            # BOOSTED: very strong wall avoidance to prevent corners
const POS_SAFE_DISTANCE := 350.0           # BOOSTED: much larger safe distance for ranged
const POS_DANGER_DISTANCE := 250.0         # BOOSTED: larger danger zone for early reaction

# Edge warning thresholds for proactive avoidance
const POS_EDGE_WARNING_DIST := 180.0       # Start avoiding edges at this distance
const POS_EDGE_DANGER_DIST := 100.0        # Extra penalty when very close to edge

# Mode Multipliers
const POS_RETREAT_THREAT_MULT := 2.0       # BOOSTED: stronger retreat priority
const POS_KITE_STRAFE_MULT := 1.5          # BOOSTED: more strafe in kite mode
const POS_APPROACH_SPEED_MULT := 1.14      # Optimized: speed in approach mode
