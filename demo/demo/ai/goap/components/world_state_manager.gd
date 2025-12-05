## World State Manager
## Manages GOAP world state synchronization to blackboard
## Emits signals when key facts change for goal evaluation
## Supports agent-vs-agent combat with dynamic threat detection
class_name WorldStateManager
extends Node

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

signal threat_changed(is_threatened: bool)
signal health_state_changed(is_low_health: bool)
signal target_killed
signal melee_threat_changed(is_melee_threat: bool)
signal ranged_threat_changed(is_ranged_threat: bool)

@export var bt_player: BTPlayer
@export var agent: CharacterBody2D

# World references (set by parent)
var target: Node2D
var weapon_pickup: Node2D  # Legacy - kept for compatibility
var melee_weapon_pickup: Node2D  # Melee weapon pickup location
var ranged_weapon_pickup: Node2D  # Ranged weapon pickup location
var ammo_pickup: Node2D
var health_pickup: Node2D

# Multiple cover objects support
var cover_objects: Array[Node2D] = []
var current_cover: Node2D  # Best cover against current target

# Multiple enemies for team battles
var enemies: Array[Node2D] = []

# Cached previous states for change detection
var _prev_under_threat := false
var _prev_low_health := false
var _prev_in_cover := false
var _prev_melee_threat := false
var _prev_ranged_threat := false

# Engagement window - suppress threat detection briefly after leaving cover
# This allows the agent to actually fight instead of immediately retreating
const ENGAGEMENT_WINDOW := 2.0
var _engagement_timer := 0.0


func _physics_process(delta: float) -> void:
	# Update engagement timer
	if _engagement_timer > 0.0:
		_engagement_timer -= delta

	_sync_world_state()


var _debug_frames := 0

func _sync_world_state() -> void:
	var bb := _get_blackboard()
	if not bb or not agent:
		return

	# Debug output for first few frames to track initialization
	_debug_frames += 1
	if _debug_frames <= 3 or _debug_frames == 10:
		print("GOAP WorldState [%s] frame %d: target=%s, melee_pickup=%s, ranged_pickup=%s" % [
			agent.name if agent else "?",
			_debug_frames,
			target.name if target else "null",
			melee_weapon_pickup.name if melee_weapon_pickup else "null",
			ranged_weapon_pickup.name if ranged_weapon_pickup else "null"
		])

	# Print key GOAP facts at frame 10 to debug planning
	if _debug_frames == 10:
		var facts := _compute_facts()
		print("GOAP Facts [%s]: has_weapon=%s, melee_avail=%s, ranged_avail=%s, target_dead=%s, in_cover=%s" % [
			agent.name,
			facts.get("has_weapon", "?"),
			facts.get("melee_weapon_available", "?"),
			facts.get("ranged_weapon_available", "?"),
			facts.get("target_dead", "?"),
			facts.get("in_cover", "?")
		])
		print("GOAP Threats [%s]: under_threat=%s, melee_threat=%s, ranged_threat=%s, enemy_melee=%s, enemy_ranged=%s" % [
			agent.name,
			facts.get("under_threat", "?"),
			facts.get("melee_threat", "?"),
			facts.get("ranged_threat", "?"),
			facts.get("enemy_has_melee_weapon", "?"),
			facts.get("enemy_has_ranged_weapon", "?")
		])

	# Track cover state changes for engagement window
	var current_in_cover: bool = agent.in_cover if "in_cover" in agent else false
	if _prev_in_cover and not current_in_cover:
		# Just left cover - start engagement window
		_engagement_timer = ENGAGEMENT_WINDOW
		print("GOAP: Engagement window started (%.1fs)" % ENGAGEMENT_WINDOW)
	_prev_in_cover = current_in_cover

	# Compute all facts
	var facts := _compute_facts()

	# Detect state changes BEFORE updating blackboard
	var under_threat: bool = facts.get("under_threat", false)
	var low_health: bool = facts.get("low_health", false)
	var melee_threat: bool = facts.get("melee_threat", false)
	var ranged_threat: bool = facts.get("ranged_threat", false)

	# Sync all facts to blackboard
	for fact_name in facts:
		bb.set_var(StringName(fact_name), facts[fact_name])

	# Emit signals for state changes
	if under_threat != _prev_under_threat:
		_prev_under_threat = under_threat
		threat_changed.emit(under_threat)

	if low_health != _prev_low_health:
		_prev_low_health = low_health
		health_state_changed.emit(low_health)

	if melee_threat != _prev_melee_threat:
		_prev_melee_threat = melee_threat
		melee_threat_changed.emit(melee_threat)

	if ranged_threat != _prev_ranged_threat:
		_prev_ranged_threat = ranged_threat
		ranged_threat_changed.emit(ranged_threat)


func _compute_facts() -> Dictionary:
	var facts := {}

	# Get agent state from agent (assumes agent has these properties/methods)
	var has_weapon: bool = agent.has_weapon if "has_weapon" in agent else false
	var ammo_count: int = agent.ammo_count if "ammo_count" in agent else 0
	var health: int = agent.health if "health" in agent else 100
	var max_health: int = agent.max_health if "max_health" in agent else 100
	var in_cover: bool = agent.in_cover if "in_cover" in agent else false
	var weapon_jammed: bool = agent.weapon_jammed if "weapon_jammed" in agent else false

	# Proximity and availability checks
	var proximity := _compute_proximity_facts()
	var target_facts := _compute_target_facts()

	# Derive states
	var has_ammo := ammo_count > 0
	var low_health: bool = health < GOAPConfigClass.LOW_HEALTH_THRESHOLD
	var is_healthy: bool = health >= GOAPConfigClass.HEALTHY_THRESHOLD

	# Weapon type from combat component
	var has_melee_weapon := false
	var has_ranged_weapon := false
	var is_suppressed := false
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("is_melee"):
			has_melee_weapon = combat.is_melee()
		if combat.has_method("is_ranged"):
			has_ranged_weapon = combat.is_ranged()
		if "is_suppressed" in combat:
			is_suppressed = combat.is_suppressed

	# Weapon loaded state: ranged needs ammo, melee is always "loaded"
	var weapon_loaded := (has_weapon and has_ammo) or has_melee_weapon

	# Distance-based facts for weapon type behavior
	var target_too_close := false  # Ranged wants to retreat
	var target_too_far := false    # Melee wants to close
	if target and is_instance_valid(target):
		var dist := agent.global_position.distance_to(target.global_position)
		target_too_close = dist < GOAPConfigClass.TOO_CLOSE_THRESHOLD
		target_too_far = dist > GOAPConfigClass.TOO_FAR_THRESHOLD

	# Enemy weapon detection - what is our opponent armed with?
	var enemy_has_melee_weapon := false
	var enemy_has_ranged_weapon := false
	if target and is_instance_valid(target) and target.has_node("CombatComponent"):
		var target_combat = target.get_node("CombatComponent")
		if target_combat.has_method("is_melee"):
			enemy_has_melee_weapon = target_combat.is_melee()
		if target_combat.has_method("is_ranged"):
			enemy_has_ranged_weapon = target_combat.is_ranged()

	# Threat detection - works for both static enemies and GOAP agents
	# Now returns detailed info: { melee_threat, ranged_threat, under_threat }
	var threat_state := _compute_threat_state()
	var under_threat: bool = threat_state.get("under_threat", false)
	var melee_threat: bool = threat_state.get("melee_threat", false)
	var ranged_threat: bool = threat_state.get("ranged_threat", false)

	# Core agent facts
	facts["has_weapon"] = has_weapon
	facts["has_ammo"] = has_ammo
	facts["weapon_loaded"] = weapon_loaded
	facts["ammo_count"] = ammo_count
	facts["health"] = health
	facts["low_health"] = low_health
	facts["is_healthy"] = is_healthy
	facts["in_cover"] = in_cover
	facts["weapon_jammed"] = weapon_jammed
	facts["under_threat"] = under_threat
	facts["enemy_attacking"] = under_threat  # Alias for compatibility
	facts["melee_threat"] = melee_threat
	facts["ranged_threat"] = ranged_threat

	# Weapon type facts
	facts["has_melee_weapon"] = has_melee_weapon
	facts["has_ranged_weapon"] = has_ranged_weapon
	facts["is_suppressed"] = is_suppressed
	facts["target_too_close"] = target_too_close
	facts["target_too_far"] = target_too_far

	# Enemy weapon facts - what the opponent has
	facts["enemy_has_melee_weapon"] = enemy_has_melee_weapon
	facts["enemy_has_ranged_weapon"] = enemy_has_ranged_weapon

	# Merge proximity facts
	for key in proximity:
		facts[key] = proximity[key]

	# Merge target facts
	for key in target_facts:
		facts[key] = target_facts[key]

	# Object references for action tasks
	facts["target"] = target
	facts["weapon_pickup"] = weapon_pickup
	facts["ammo_pickup"] = ammo_pickup
	facts["health_pickup"] = health_pickup
	facts["cover_object"] = current_cover  # Best cover against target

	return facts


## Computes detailed threat state - melee and ranged threats separately
## Returns a dictionary with { melee_threat, ranged_threat, under_threat }
## This allows tactical decisions based on what KIND of threat we're facing
func _compute_threat_state() -> Dictionary:
	var result := {
		"melee_threat": false,
		"ranged_threat": false,
		"under_threat": false
	}

	if not target or not is_instance_valid(target):
		return result

	# If we're in cover, ranged threat is negated but melee threat remains
	# Cover protects from projectiles, not from someone walking up and stabbing you
	var in_cover: bool = agent.in_cover if "in_cover" in agent else false

	# Engagement window: suppress threat detection briefly after leaving cover
	# This allows the agent to actually fight instead of immediately retreating
	if _engagement_timer > 0.0:
		return result

	# Check for legacy telegraph-based enemies
	if target.has_method("is_preparing_attack"):
		if target.is_preparing_attack():
			result["ranged_threat"] = true
			result["under_threat"] = true
			return result

	# Distance-based threat detection for GOAP agents
	if _is_goap_agent(target):
		var dist := agent.global_position.distance_to(target.global_position)
		var target_has_weapon: bool = target.has_weapon if "has_weapon" in target else false

		if not target_has_weapon:
			return result

		# Get target weapon type
		var target_has_melee := false
		var target_has_ranged := false
		if target.has_node("CombatComponent"):
			var target_combat = target.get_node("CombatComponent")
			if target_combat.has_method("is_melee"):
				target_has_melee = target_combat.is_melee()
			if target_combat.has_method("is_ranged"):
				target_has_ranged = target_combat.is_ranged()

		# --- MELEE THREAT ---
		# Melee is a threat when enemy is close AND has melee weapon
		# Cover does NOT protect against melee
		if target_has_melee:
			var in_melee_threat_range := dist < GOAPConfigClass.TOO_CLOSE_THRESHOLD
			if in_melee_threat_range:
				result["melee_threat"] = true
				result["under_threat"] = true

			# Extended threat range for ranged agents without ammo
			# A defenseless ranged agent should feel melee threat from farther away
			if not result["melee_threat"]:
				var agent_has_ranged := false
				if agent.has_node("CombatComponent"):
					var agent_combat = agent.get_node("CombatComponent")
					if agent_combat.has_method("is_ranged"):
						agent_has_ranged = agent_combat.is_ranged()

				if agent_has_ranged:
					var agent_ammo: int = agent.ammo_count if "ammo_count" in agent else 0
					var agent_jammed: bool = agent.weapon_jammed if "weapon_jammed" in agent else false
					var agent_cant_shoot := agent_ammo <= 0 or agent_jammed

					if agent_cant_shoot:
						# Defenseless ranged agent - melee is scary even from mid-range
						var in_extended_threat_range := dist < GOAPConfigClass.SHOOTING_RANGE
						if in_extended_threat_range:
							result["melee_threat"] = true
							result["under_threat"] = true

		# --- RANGED THREAT ---
		# Ranged is a threat when enemy has loaded ranged weapon, is in range, and has LoS
		# Being in cover NEGATES ranged threat (cover works!)
		if target_has_ranged and not in_cover:
			# High health agents are more confident against ranged fire
			var health: int = agent.health if "health" in agent else 100
			var max_health: int = agent.max_health if "max_health" in agent else 100
			var health_ratio := float(health) / float(max_health)

			# Only feel threatened by ranged if health is below 70%
			if health_ratio <= 0.7:
				var in_shooting_range := dist < GOAPConfigClass.SHOOTING_RANGE
				if in_shooting_range:
					var target_ammo: int = target.ammo_count if "ammo_count" in target else 0
					var target_jammed: bool = target.weapon_jammed if "weapon_jammed" in target else false
					var target_weapon_ready := target_ammo > 0 and not target_jammed

					if target_weapon_ready:
						var target_has_los := _target_has_los_to_us()
						if target_has_los:
							result["ranged_threat"] = true
							result["under_threat"] = true

	return result


## Checks if a node is a GOAP agent (has the expected properties)
func _is_goap_agent(node: Node2D) -> bool:
	return "has_weapon" in node and "ammo_count" in node and "health" in node


## Checks if the target has line of sight to this agent
func _target_has_los_to_us() -> bool:
	if not target or not is_instance_valid(target):
		return false

	var space_state := agent.get_world_2d().direct_space_state
	var query := PhysicsRayQueryParameters2D.create(
		target.global_position,
		agent.global_position,
		GOAPConfigClass.LOS_COLLISION_LAYER
	)
	var result := space_state.intersect_ray(query)

	return result.is_empty()


func _compute_proximity_facts() -> Dictionary:
	var facts := {}
	var agent_pos: Vector2 = agent.global_position

	# Weapon pickup (legacy - for backwards compatibility)
	var near_weapon_pickup := false
	var weapon_available := false
	if weapon_pickup and is_instance_valid(weapon_pickup):
		weapon_available = _is_pickup_available(weapon_pickup)
		if weapon_available:
			near_weapon_pickup = agent_pos.distance_to(weapon_pickup.global_position) < GOAPConfigClass.PICKUP_RANGE

	# Melee weapon pickup
	var melee_weapon_available := false
	if melee_weapon_pickup and is_instance_valid(melee_weapon_pickup):
		melee_weapon_available = _is_pickup_available(melee_weapon_pickup)

	# Ranged weapon pickup
	var ranged_weapon_available := false
	if ranged_weapon_pickup and is_instance_valid(ranged_weapon_pickup):
		ranged_weapon_available = _is_pickup_available(ranged_weapon_pickup)

	# Unified weapon_available - true if ANY weapon pickup is available
	# This allows the planner to use a single PickUpWeapon action
	var any_weapon_available := melee_weapon_available or ranged_weapon_available or weapon_available

	# Ammo pickup
	var near_ammo := false
	var ammo_available := false
	if ammo_pickup and is_instance_valid(ammo_pickup):
		ammo_available = _is_pickup_available(ammo_pickup)
		if ammo_available:
			near_ammo = agent_pos.distance_to(ammo_pickup.global_position) < GOAPConfigClass.PICKUP_RANGE

	# Health pickup
	var near_health_pickup := false
	var health_available := false
	if health_pickup and is_instance_valid(health_pickup):
		health_available = _is_pickup_available(health_pickup)
		if health_available:
			near_health_pickup = agent_pos.distance_to(health_pickup.global_position) < GOAPConfigClass.PICKUP_RANGE

	# Find best cover against current target
	_update_best_cover()

	# Cover proximity (to best cover)
	var near_cover := false
	if current_cover and is_instance_valid(current_cover):
		near_cover = agent_pos.distance_to(current_cover.global_position) < GOAPConfigClass.COVER_RANGE

	facts["near_weapon_pickup"] = near_weapon_pickup
	facts["weapon_available"] = any_weapon_available  # Use unified weapon_available
	facts["melee_weapon_available"] = melee_weapon_available
	facts["ranged_weapon_available"] = ranged_weapon_available
	facts["near_ammo"] = near_ammo
	facts["ammo_available"] = ammo_available
	facts["near_health_pickup"] = near_health_pickup
	facts["health_available"] = health_available
	facts["near_cover"] = near_cover

	# Weapon pickup references for behavior trees
	facts["melee_weapon_pickup"] = melee_weapon_pickup
	facts["ranged_weapon_pickup"] = ranged_weapon_pickup

	return facts


## Finds the best cover object that blocks LoS from the target
func _update_best_cover() -> void:
	if cover_objects.is_empty():
		current_cover = null
		return

	if not target or not is_instance_valid(target):
		# No target - use nearest cover
		current_cover = _find_nearest_cover()
		return

	var best_cover: Node2D = null
	var best_score := -INF

	for cover in cover_objects:
		if not is_instance_valid(cover):
			continue

		var score := _evaluate_cover(cover)
		if score > best_score:
			best_score = score
			best_cover = cover

	current_cover = best_cover


## Evaluates how good a cover object is against the current target
## Higher score = better cover
func _evaluate_cover(cover: Node2D) -> float:
	var agent_pos := agent.global_position
	var cover_pos := cover.global_position

	# Base score: prefer closer covers
	var dist_to_cover := agent_pos.distance_to(cover_pos)
	var dist_score := 1000.0 - dist_to_cover  # Closer = higher score

	# Bonus: cover that blocks LoS from target
	if target and is_instance_valid(target):
		var cover_position := _get_cover_position_for(cover)
		if _position_blocks_los_from_target(cover_position, cover):
			dist_score += 500.0  # Big bonus for actual cover

	return dist_score


## Gets the position an agent should move to for cover behind this object
func _get_cover_position_for(cover: Node2D) -> Vector2:
	if cover.has_method("get_cover_position_against") and target:
		return cover.get_cover_position_against(target.global_position)
	return cover.global_position


## Checks if a position is protected from target's LoS by a cover object
func _position_blocks_los_from_target(pos: Vector2, cover: Node2D) -> bool:
	if not target or not is_instance_valid(target):
		return false

	var space_state := agent.get_world_2d().direct_space_state
	var query := PhysicsRayQueryParameters2D.create(
		target.global_position,
		pos,
		GOAPConfigClass.LOS_COLLISION_LAYER
	)
	var result := space_state.intersect_ray(query)

	# Check if the ray hit the cover object
	if not result.is_empty():
		var hit_collider = result.get("collider")
		if hit_collider == cover or (cover.has_node("StaticBody2D") and hit_collider == cover.get_node("StaticBody2D")):
			return true

	return false


## Finds the nearest cover object
func _find_nearest_cover() -> Node2D:
	var nearest: Node2D = null
	var nearest_dist := INF

	for cover in cover_objects:
		if not is_instance_valid(cover):
			continue
		var dist := agent.global_position.distance_to(cover.global_position)
		if dist < nearest_dist:
			nearest_dist = dist
			nearest = cover

	return nearest


func _compute_target_facts() -> Dictionary:
	var facts := {}

	var target_in_range := false
	var target_in_sight := false
	var target_visible := false
	var target_dead := false
	var target_in_cover := false

	if target == null or not is_instance_valid(target):
		target_dead = true
	else:
		var dist := agent.global_position.distance_to(target.global_position)
		target_in_range = dist < GOAPConfigClass.ATTACK_RANGE
		target_in_sight = dist < GOAPConfigClass.SHOOTING_RANGE
		target_visible = _has_los_to_target()

		# Check if target is in cover (for GOAP agents)
		if "in_cover" in target:
			target_in_cover = target.in_cover

		# Check if target is dead - works for both enemy types and GOAP agents
		if target.has_node("Health"):
			var health_node = target.get_node("Health")
			if health_node.has_method("get_current"):
				target_dead = health_node.get_current() <= 0
		elif "health" in target:
			# GOAP agent - check health property directly
			target_dead = target.health <= 0

	facts["target_in_range"] = target_in_range
	facts["target_in_sight"] = target_in_sight
	facts["target_visible"] = target_visible
	facts["target_dead"] = target_dead
	facts["target_in_cover"] = target_in_cover

	return facts


func _is_pickup_available(pickup: Node2D) -> bool:
	if pickup.has_method("is_available"):
		return pickup.is_available()
	return true  # Fallback for pickups without availability check


func _has_los_to_target() -> bool:
	if not target or not is_instance_valid(target):
		return false

	var space_state := agent.get_world_2d().direct_space_state
	# Check from projectile spawn position (matches goap_attack.gd spawn offset)
	# Projectiles spawn 60 pixels toward target + 40 pixels up
	var dir_to_target := (target.global_position - agent.global_position).normalized()
	var spawn_offset := dir_to_target * 60.0
	var from_pos := agent.global_position + spawn_offset + Vector2(0, -40)
	var query := PhysicsRayQueryParameters2D.create(
		from_pos,
		target.global_position,
		GOAPConfigClass.LOS_COLLISION_LAYER
	)
	var result := space_state.intersect_ray(query)

	return result.is_empty()


func _get_blackboard() -> Blackboard:
	if bt_player:
		return bt_player.get_blackboard()
	return null


## Force immediate state sync (useful after agent state changes)
func force_sync() -> void:
	_sync_world_state()


## Sets the cover objects array (called by parent agent)
func set_cover_objects(covers: Array) -> void:
	cover_objects.clear()
	for cover in covers:
		if cover is Node2D:
			cover_objects.append(cover)
