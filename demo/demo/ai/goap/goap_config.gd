## GOAP Demo Configuration
## Centralized constants for the GOAP tactical demo
class_name GOAPConfig
extends RefCounted

# Combat ranges
const ATTACK_RANGE := 150.0       # Melee attack range
const SHOOTING_RANGE := 800.0     # Ranged weapon range
const PICKUP_RANGE := 80.0        # Distance to collect pickups
const COVER_RANGE := 100.0        # Distance to be considered "at cover"

# Arena bounds (keeps agents inside playable area - extended for more maneuvering room)
const ARENA_MIN := Vector2(50.0, 50.0)
const ARENA_MAX := Vector2(1870.0, 1030.0)

# Edge awareness thresholds
const EDGE_WARNING_DISTANCE := 150.0   # Distance from edge to start considering repositioning
const EDGE_DANGER_DISTANCE := 80.0     # Distance at which agent is cornered/trapped

# Movement - weapon-specific speeds (ranged faster for kiting)
const MOVE_SPEED := 300.0
const RANGED_MOVE_SPEED := 350.0
const MELEE_MOVE_SPEED := 280.0

# Melee attack balance
const MELEE_SWING_COOLDOWN := 0.8     # Time between melee swings
const MELEE_SWING_SLOW_FACTOR := 0.3  # Speed multiplier during swing (30% speed)

# Health thresholds
const LOW_HEALTH_THRESHOLD := 50
const HEALTHY_THRESHOLD := 80

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

# Weapon type combat distances
const RETREAT_DISTANCE := 300.0        # Ranged wants to be this far away
const CLOSE_GAP_THRESHOLD := 150.0     # Melee wants to be within this distance
const TOO_CLOSE_THRESHOLD := 200.0     # Ranged feels threatened within this distance
const TOO_FAR_THRESHOLD := 150.0       # Melee needs to close if farther than this

# Suppression
const SUPPRESSION_DURATION := 1.5      # Duration in seconds

# Accuracy
const COVER_ACCURACY_PENALTY := 0.5    # 50% accuracy when shooting from cover

# ============================================
# OPTIMIZED GOAP WEIGHTS
# These values were discovered through evolutionary optimization.
# Run optimization_runner.tscn to discover better values for your game!
# ============================================

# Action Base Costs (lower = preferred by planner)
const COST_GO_TO_WEAPON := 3
const COST_PICKUP_WEAPON := 1
const COST_GO_TO_AMMO := 3
const COST_LOAD_WEAPON := 1
const COST_GO_TO_COVER := 2
const COST_TAKE_COVER := 1
const COST_LEAVE_COVER := 1
const COST_APPROACH_TARGET := 4
const COST_FLANK := 5
const COST_SHOOT := 1
const COST_MELEE_ATTACK := 6       # Optimized: was 8, lowered to encourage melee
const COST_RETREAT := 2            # Optimized: was 3, lowered for faster escapes

# Dynamic Cost Modifiers
const MOD_PREFERRED_WEAPON_BONUS := -4     # Optimized: stronger preference for favored weapon
const MOD_NONPREFERRED_WEAPON_PENALTY := 6 # Optimized: stronger penalty for wrong weapon
const MOD_CLOSE_PICKUP_BONUS := -3         # Optimized: prioritize nearby pickups
const MOD_FAR_PICKUP_PENALTY := 4
const MOD_COUNTER_WEAPON_BONUS := -3       # Optimized: reward counter-picking
const MOD_SAME_WEAPON_PENALTY := 2
const MOD_ENEMY_CLOSE_MELEE_BONUS := -4    # Optimized: stronger melee bonus when close
const MOD_ENEMY_FAR_MELEE_PENALTY := 3
const MOD_NO_AMMO_MELEE_BONUS := -3        # Optimized: melee attractive without ammo

# Threat-Based Cost Overrides
const COST_COVER_VS_MELEE := 120           # Optimized: even higher to discourage cover vs melee
const COST_COVER_VS_RANGED := 1            # Cover is great vs ranged
const COST_RETREAT_VS_MELEE := 1           # Retreat is critical vs melee
const COST_AMMO_WITH_MELEE := 1000         # Melee doesn't need ammo

# Goal Timing (Hysteresis) - Optimized for faster reactions
const GOAL_SWITCH_COOLDOWN := 0.4          # Optimized: slightly faster switching
const DEFENSIVE_GOAL_COMMITMENT := 0.8     # Optimized: shorter defensive phases
const ATTACK_GOAL_COMMITMENT := 0.25       # Optimized: quicker to switch from attack
const DEFENSIVE_TIMEOUT := 3.0             # Optimized: faster return to attack mode

# ============================================
# CONTEXT-AWARE ACTION COSTS
# These modifiers adjust action costs based on tactical context
# ============================================

# Flank Action - cheaper when target is in cover (flanking useful)
const COST_FLANK_TARGET_IN_COVER := -3     # Bonus when flanking hidden target
const COST_FLANK_TARGET_EXPOSED := 4       # Penalty when target already exposed
const COST_FLANK_LOW_HEALTH := 3           # Penalty for risky flanking when hurt

# Kite Attack - ideal for ranged vs melee
const COST_KITE_VS_MELEE := -4             # Bonus vs melee threat (kiting is ideal)
const COST_KITE_LOW_AMMO := 5              # Penalty when ammo is scarce
const COST_KITE_HIGH_HEALTH := -2          # Bonus when healthy (can trade shots)

# Close Gap - for melee agents
const COST_CLOSE_VS_RANGED := 4            # Penalty vs ranged enemy (risky approach)
const COST_CLOSE_LOW_HEALTH := 5           # Penalty when health is critical
const COST_CLOSE_TARGET_WOUNDED := -3      # Bonus when target is hurt (finish them!)

# Approach Target - general movement toward enemy
const COST_APPROACH_TARGET_IN_COVER := 3   # Penalty vs entrenched enemy
const COST_APPROACH_WOUNDED_TARGET := -2   # Bonus vs wounded target
const COST_APPROACH_RECENT_DAMAGE := 4     # Penalty if recently took damage

# Take Cover - defensive positioning
const COST_TAKE_COVER_MELEE_RANGE := 6     # Penalty in melee range (escape first!)
const COST_TAKE_COVER_EXPOSED := -3        # Bonus when exposed with ranged threat
const COST_TAKE_COVER_LOW_HEALTH := -4     # Bonus when health is low

# Leave Cover - exiting safety
const COST_LEAVE_COVER_RANGED_THREAT := 5  # Penalty with active ranged threat
const COST_LEAVE_COVER_MELEE_THREAT := -3  # Bonus to escape melee (cover won't help)
const COST_LEAVE_COVER_HIGH_HEALTH := -2   # Bonus when healthy (can take hits)

# Wait In Cover - staying put
const COST_WAIT_LOW_AMMO := 4              # Penalty to encourage reloading
const COST_WAIT_LOW_HEALTH := 5            # Penalty to encourage healing
const COST_WAIT_LONG_COMBAT := 3           # Penalty as combat drags on

# Heal Action - seeking health pickups
const COST_HEAL_DURING_THREAT := 4         # Penalty when actively threatened
const COST_HEAL_HEALTH_SCALE := 1.0        # Scale by health gap (higher = cheaper)
const COST_HEAL_COMBAT_ACTIVE := 3         # Penalty in active combat

# Shoot Action - firing ranged weapons
const COST_SHOOT_LAST_AMMO := 4            # Penalty for using final shot
const COST_SHOOT_TARGET_IN_COVER := 3      # Penalty when target is in cover
const COST_SHOOT_HIGH_ACCURACY := -2       # Bonus when target is exposed/close

# Melee Attack - close combat
const COST_MELEE_VS_RANGED := -3           # Bonus when enemy has ranged (counter)
const COST_MELEE_VS_MELEE := 2             # Penalty for melee vs melee (fair fight)
const COST_MELEE_LOW_HEALTH := 4           # Penalty when low health (risky)

# Global Modifiers
const HEALTH_COST_SCALING := 1.0           # How much health affects all costs
const AMMO_SCARCITY_THRESHOLD := 0.3       # Below this triggers conservation mode
const ENGAGEMENT_WINDOW := 2.0             # Seconds of engagement window for timing
