## GOAP Demo Configuration
## Centralized constants for the GOAP tactical demo
class_name GOAPConfig
extends RefCounted

# Combat ranges
const ATTACK_RANGE := 150.0       # Melee attack range
const SHOOTING_RANGE := 800.0     # Ranged weapon range
const PICKUP_RANGE := 80.0        # Distance to collect pickups
const COVER_RANGE := 100.0        # Distance to be considered "at cover"

# Arena bounds (keeps agents inside playable area)
const ARENA_MIN := Vector2(100.0, 150.0)
const ARENA_MAX := Vector2(1300.0, 600.0)

# Movement
const MOVE_SPEED := 300.0

# Health thresholds
const LOW_HEALTH_THRESHOLD := 50
const HEALTHY_THRESHOLD := 80

# Resource defaults
const DEFAULT_MAX_AMMO := 10
const DEFAULT_MAX_HEALTH := 100
const DEFAULT_JAM_CHANCE := 0.15

# Scarcity mode settings
const SCARCITY_MAX_AMMO := 3
const SCARCITY_AMMO_RESPAWN := 12.0
const SCARCITY_HEALTH_RESPAWN := 15.0
const NORMAL_AMMO_RESPAWN := 5.0
const NORMAL_HEALTH_RESPAWN := 8.0

# Physics layers
const LOS_COLLISION_LAYER := 16
