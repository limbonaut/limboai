## GOAP Demo Configuration
## Centralized constants for the GOAP tactical demo
class_name GOAPConfig
extends RefCounted

# Combat ranges
const ATTACK_RANGE := 150.0       # Melee attack range
const SHOOTING_RANGE := 800.0     # Ranged weapon range
const PICKUP_RANGE := 80.0        # Distance to collect pickups
const COVER_RANGE := 100.0        # Distance to be considered "at cover"

# Arena bounds (keeps agents inside playable area - matches 1920x1080 with margins)
const ARENA_MIN := Vector2(80.0, 80.0)
const ARENA_MAX := Vector2(1840.0, 1000.0)

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
