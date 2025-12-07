## Position Evaluator
## Scores candidate positions using optimizable weights.
## Used by movement tasks to make positioning decisions that the optimizer can tune.
##
## The evaluator generates candidate positions around the agent and scores each
## based on multiple factors (distance to threat, resources, cover, etc.).
## All weights are exposed for evolutionary optimization.
class_name PositionEvaluator
extends Node

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const ArenaUtilityClass = preload("res://demo/ai/goap/arena_utility.gd")
const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")

# ============================================
# OPTIMIZABLE WEIGHTS (set by WeightApplicator)
# ============================================

## Weight for distance from threat (higher = prefer farther from threat)
var weight_threat_distance := 1.0

## Weight for proximity to ammo pickup (higher = prefer closer to ammo)
var weight_ammo_proximity := 0.5

## Weight for proximity to health pickup (higher = prefer closer to health)
var weight_health_proximity := 0.3

## Weight for proximity to cover (higher = prefer closer to cover)
var weight_cover_proximity := 0.4

## Weight for proximity to arena center (higher = prefer center)
var weight_center_proximity := 0.2

## Weight for strafe movement (higher = prefer perpendicular to threat)
var weight_strafe_preference := 0.6

## Weight for maintaining line of sight to target
var weight_los_to_target := 0.3

## Weight for proximity to speed boost pickup
var weight_speed_boost_proximity := 0.2

# ============================================
# URGENCY SCALING
# ============================================

## Multiplier for ammo weight when ammo is low
var ammo_urgency_scale := 1.5

## Multiplier for health weight when health is low
var health_urgency_scale := 2.0

## Ammo ratio below which urgency kicks in (0.0-1.0)
var low_ammo_threshold := 0.3

## Health ratio below which urgency kicks in (0.0-1.0)
var low_health_threshold := 0.4

# ============================================
# MOVEMENT PARAMETERS
# ============================================

## Distance for candidate position generation
var step_distance := 100.0

## Wall avoidance strength (0-1)
var wall_avoidance := 0.5

## Distance considered "safe" from threat
var safe_distance := 250.0

## Distance considered "dangerous" from threat
var danger_distance := 120.0

# ============================================
# MODE MULTIPLIERS
# ============================================

## Threat weight multiplier in retreat mode
var retreat_threat_mult := 2.0

## Strafe weight multiplier in kite mode
var kite_strafe_mult := 1.5

## Speed multiplier in approach mode
var approach_speed_mult := 1.0

# ============================================
# WEAPON-TYPE MULTIPLIERS
# Applied based on whether agent has ranged or melee weapon
# ============================================

## Multipliers for ranged weapon users
var ranged_threat_distance_mult := GOAPConfigClass.RANGED_THREAT_DISTANCE_MULT
var ranged_ammo_proximity_mult := GOAPConfigClass.RANGED_AMMO_PROXIMITY_MULT
var ranged_health_proximity_mult := GOAPConfigClass.RANGED_HEALTH_PROXIMITY_MULT
var ranged_cover_proximity_mult := GOAPConfigClass.RANGED_COVER_PROXIMITY_MULT
var ranged_center_proximity_mult := GOAPConfigClass.RANGED_CENTER_PROXIMITY_MULT
var ranged_strafe_preference_mult := GOAPConfigClass.RANGED_STRAFE_PREFERENCE_MULT
var ranged_safe_distance_mult := GOAPConfigClass.RANGED_SAFE_DISTANCE_MULT
var ranged_danger_distance_mult := GOAPConfigClass.RANGED_DANGER_DISTANCE_MULT

## Multipliers for melee weapon users
var melee_threat_distance_mult := GOAPConfigClass.MELEE_THREAT_DISTANCE_MULT
var melee_ammo_proximity_mult := GOAPConfigClass.MELEE_AMMO_PROXIMITY_MULT
var melee_health_proximity_mult := GOAPConfigClass.MELEE_HEALTH_PROXIMITY_MULT
var melee_cover_proximity_mult := GOAPConfigClass.MELEE_COVER_PROXIMITY_MULT
var melee_center_proximity_mult := GOAPConfigClass.MELEE_CENTER_PROXIMITY_MULT
var melee_strafe_preference_mult := GOAPConfigClass.MELEE_STRAFE_PREFERENCE_MULT
var melee_safe_distance_mult := GOAPConfigClass.MELEE_SAFE_DISTANCE_MULT
var melee_danger_distance_mult := GOAPConfigClass.MELEE_DANGER_DISTANCE_MULT

## Whether the agent currently has a ranged weapon (set before scoring)
var _is_ranged_weapon := false


## Returns the effective weight adjusted for weapon type
func _get_weapon_adjusted_weight(base_weight: float, ranged_mult: float, melee_mult: float) -> float:
	if _is_ranged_weapon:
		return base_weight * ranged_mult
	else:
		return base_weight * melee_mult


## Sets the weapon type for position scoring
## Call this before get_best_position() to enable weapon-specific behavior
func set_weapon_type(is_ranged: bool) -> void:
	_is_ranged_weapon = is_ranged


## Generates candidate positions around the agent
## Returns array of valid positions within arena bounds
func generate_candidates(from_pos: Vector2, context: Dictionary) -> Array[Vector2]:
	var candidates: Array[Vector2] = []

	# 8 compass directions
	var directions := [
		Vector2.UP,
		Vector2.DOWN,
		Vector2.LEFT,
		Vector2.RIGHT,
		Vector2(1, 1).normalized(),   # Down-right
		Vector2(-1, 1).normalized(),  # Down-left
		Vector2(1, -1).normalized(),  # Up-right
		Vector2(-1, -1).normalized(), # Up-left
	]

	for dir: Vector2 in directions:
		var candidate: Vector2 = from_pos + dir * step_distance
		if ArenaUtilityClass.is_position_in_bounds(candidate, 20.0):
			candidates.append(candidate)

	# Add resource-directed positions
	var threat_pos: Vector2 = context.get("threat_pos", Vector2.ZERO)
	var ammo_pos: Vector2 = context.get("ammo_pos", Vector2.ZERO)
	var health_pos: Vector2 = context.get("health_pos", Vector2.ZERO)
	var cover_pos: Vector2 = context.get("cover_pos", Vector2.ZERO)
	var speed_boost_pos: Vector2 = context.get("speed_boost_pos", Vector2.ZERO)
	var center := ArenaUtilityClass.get_arena_center()

	# Direction toward each resource (scaled by step_distance)
	if ammo_pos != Vector2.ZERO:
		var dir_to_ammo := (ammo_pos - from_pos).normalized()
		var toward_ammo := from_pos + dir_to_ammo * step_distance
		if ArenaUtilityClass.is_position_in_bounds(toward_ammo, 20.0):
			candidates.append(toward_ammo)

	if health_pos != Vector2.ZERO:
		var dir_to_health := (health_pos - from_pos).normalized()
		var toward_health := from_pos + dir_to_health * step_distance
		if ArenaUtilityClass.is_position_in_bounds(toward_health, 20.0):
			candidates.append(toward_health)

	if cover_pos != Vector2.ZERO:
		var dir_to_cover := (cover_pos - from_pos).normalized()
		var toward_cover := from_pos + dir_to_cover * step_distance
		if ArenaUtilityClass.is_position_in_bounds(toward_cover, 20.0):
			candidates.append(toward_cover)

	if speed_boost_pos != Vector2.ZERO:
		var dir_to_boost := (speed_boost_pos - from_pos).normalized()
		var toward_boost := from_pos + dir_to_boost * step_distance
		if ArenaUtilityClass.is_position_in_bounds(toward_boost, 20.0):
			candidates.append(toward_boost)

	# Direction toward center
	var dir_to_center := (center - from_pos).normalized()
	var toward_center := from_pos + dir_to_center * step_distance
	if ArenaUtilityClass.is_position_in_bounds(toward_center, 20.0):
		candidates.append(toward_center)

	# Strafe directions (perpendicular to threat)
	if threat_pos != Vector2.ZERO:
		var to_threat := (threat_pos - from_pos).normalized()
		var strafe_left := Vector2(-to_threat.y, to_threat.x)
		var strafe_right := Vector2(to_threat.y, -to_threat.x)

		var strafe_left_pos := from_pos + strafe_left * step_distance
		var strafe_right_pos := from_pos + strafe_right * step_distance

		if ArenaUtilityClass.is_position_in_bounds(strafe_left_pos, 20.0):
			candidates.append(strafe_left_pos)
		if ArenaUtilityClass.is_position_in_bounds(strafe_right_pos, 20.0):
			candidates.append(strafe_right_pos)

	return candidates


## Scores a candidate position based on all factors
## Higher score = better position
func score_position(pos: Vector2, context: Dictionary) -> float:
	var score := 0.0

	# Extract context
	var from_pos: Vector2 = context.get("from_pos", Vector2.ZERO)
	var threat_pos: Vector2 = context.get("threat_pos", Vector2.ZERO)
	var ammo_pos: Vector2 = context.get("ammo_pos", Vector2.ZERO)
	var health_pos: Vector2 = context.get("health_pos", Vector2.ZERO)
	var cover_pos: Vector2 = context.get("cover_pos", Vector2.ZERO)
	var speed_boost_pos: Vector2 = context.get("speed_boost_pos", Vector2.ZERO)
	var ammo_ratio: float = context.get("ammo_ratio", 1.0)
	var health_ratio: float = context.get("health_ratio", 1.0)
	var has_speed_boost: bool = context.get("has_speed_boost", false)
	var center := ArenaUtilityClass.get_arena_center()

	# Calculate weapon-adjusted weights
	var eff_threat_weight := _get_weapon_adjusted_weight(
		weight_threat_distance, ranged_threat_distance_mult, melee_threat_distance_mult)
	var eff_ammo_weight := _get_weapon_adjusted_weight(
		weight_ammo_proximity, ranged_ammo_proximity_mult, melee_ammo_proximity_mult)
	var eff_health_weight := _get_weapon_adjusted_weight(
		weight_health_proximity, ranged_health_proximity_mult, melee_health_proximity_mult)
	var eff_cover_weight := _get_weapon_adjusted_weight(
		weight_cover_proximity, ranged_cover_proximity_mult, melee_cover_proximity_mult)
	var eff_center_weight := _get_weapon_adjusted_weight(
		weight_center_proximity, ranged_center_proximity_mult, melee_center_proximity_mult)
	var eff_strafe_weight := _get_weapon_adjusted_weight(
		weight_strafe_preference, ranged_strafe_preference_mult, melee_strafe_preference_mult)
	var eff_safe_distance := _get_weapon_adjusted_weight(
		safe_distance, ranged_safe_distance_mult, melee_safe_distance_mult)
	var eff_danger_distance := _get_weapon_adjusted_weight(
		danger_distance, ranged_danger_distance_mult, melee_danger_distance_mult)

	# Calculate urgency multipliers
	var ammo_urgency := 1.0
	if ammo_ratio < low_ammo_threshold:
		ammo_urgency = ammo_urgency_scale

	var health_urgency := 1.0
	if health_ratio < low_health_threshold:
		health_urgency = health_urgency_scale

	# --- THREAT DISTANCE (with prediction) ---
	if threat_pos != Vector2.ZERO:
		var threat_dist := pos.distance_to(threat_pos)
		var current_threat_dist := from_pos.distance_to(threat_pos)

		# Normalize: positive if we're getting farther, negative if closer
		var threat_delta := (threat_dist - current_threat_dist) / step_distance
		score += threat_delta * eff_threat_weight * 100.0

		# Bonus for being at safe distance
		if threat_dist >= eff_safe_distance:
			score += eff_threat_weight * 20.0
		elif threat_dist <= eff_danger_distance:
			score -= eff_threat_weight * 30.0

		# CRITICAL: Heavy penalty for moving TOWARD threat when in danger zone
		# This prevents the agent from accidentally walking into the enemy
		# Note: For melee agents (low/negative threat weight), this is less punishing
		if current_threat_dist < eff_danger_distance * 1.5:
			if threat_dist < current_threat_dist:
				# Moving closer when already close - bad for ranged, less so for melee
				var approach_penalty := (current_threat_dist - threat_dist) / step_distance
				score -= approach_penalty * eff_threat_weight * 150.0

		# --- THREAT PREDICTION: Penalize positions the threat is moving toward ---
		# This helps ranged agents path AROUND approaching melee, not into them
		var threat_velocity: Vector2 = context.get("threat_velocity", Vector2.ZERO)
		if threat_velocity.length() > 10.0:  # Only if threat is moving
			# Predict where threat will be in ~0.5 seconds
			var predicted_threat_pos := threat_pos + threat_velocity * 0.5
			var dist_to_predicted := pos.distance_to(predicted_threat_pos)
			var current_dist_to_predicted := from_pos.distance_to(predicted_threat_pos)

			# Bonus for moving away from where threat WILL be
			var prediction_delta := (dist_to_predicted - current_dist_to_predicted) / step_distance
			score += prediction_delta * eff_threat_weight * 80.0

			# Extra penalty if moving INTO threat's path
			var threat_dir := threat_velocity.normalized()
			var move_dir := (pos - from_pos).normalized()
			var path_alignment := move_dir.dot(threat_dir)
			if path_alignment > 0.3:  # Moving in same direction as threat (bad!)
				score -= path_alignment * eff_threat_weight * 60.0

			# Bonus for perpendicular movement (circling around threat)
			var perpendicular_factor := 1.0 - absf(path_alignment)
			if dist_to_predicted > current_dist_to_predicted:  # Only if also getting farther
				score += perpendicular_factor * eff_threat_weight * 40.0

	# --- AMMO PROXIMITY ---
	if ammo_pos != Vector2.ZERO:
		var ammo_dist := pos.distance_to(ammo_pos)
		var current_ammo_dist := from_pos.distance_to(ammo_pos)

		# Positive if we're getting closer to ammo
		var ammo_delta := (current_ammo_dist - ammo_dist) / step_distance
		score += ammo_delta * eff_ammo_weight * ammo_urgency * 50.0

	# --- HEALTH PROXIMITY ---
	if health_pos != Vector2.ZERO:
		var health_dist := pos.distance_to(health_pos)
		var current_health_dist := from_pos.distance_to(health_pos)

		# Positive if we're getting closer to health
		var health_delta := (current_health_dist - health_dist) / step_distance
		score += health_delta * eff_health_weight * health_urgency * 50.0

	# --- COVER PROXIMITY ---
	if cover_pos != Vector2.ZERO:
		var cover_dist := pos.distance_to(cover_pos)
		var current_cover_dist := from_pos.distance_to(cover_pos)

		var cover_delta := (current_cover_dist - cover_dist) / step_distance
		score += cover_delta * eff_cover_weight * 30.0

	# --- SPEED BOOST PROXIMITY ---
	if speed_boost_pos != Vector2.ZERO and not has_speed_boost:
		var boost_dist := pos.distance_to(speed_boost_pos)
		var current_boost_dist := from_pos.distance_to(speed_boost_pos)

		var boost_delta := (current_boost_dist - boost_dist) / step_distance
		score += boost_delta * weight_speed_boost_proximity * 40.0

	# --- CENTER PROXIMITY ---
	var center_dist := pos.distance_to(center)
	var current_center_dist := from_pos.distance_to(center)

	var center_delta := (current_center_dist - center_dist) / step_distance
	score += center_delta * eff_center_weight * 20.0

	# --- STRAFE BONUS ---
	if threat_pos != Vector2.ZERO:
		var to_threat := (threat_pos - from_pos).normalized()
		var move_dir := (pos - from_pos).normalized()

		# Strafe is perpendicular movement (dot product close to 0)
		# We want to reward movement that's neither directly toward nor away
		var dot := absf(move_dir.dot(to_threat))
		var strafe_factor := 1.0 - dot  # 1.0 = pure strafe, 0.0 = direct approach/retreat
		score += strafe_factor * eff_strafe_weight * 25.0

	# --- PROACTIVE EDGE AVOIDANCE ---
	# Apply penalties based on distance to edges - stronger when closer
	# This prevents the agent from backing into corners in the first place

	# Calculate distances to each edge
	var dist_to_left := pos.x - GOAPConfigClass.ARENA_MIN.x
	var dist_to_right := GOAPConfigClass.ARENA_MAX.x - pos.x
	var dist_to_top := pos.y - GOAPConfigClass.ARENA_MIN.y
	var dist_to_bottom := GOAPConfigClass.ARENA_MAX.y - pos.y
	var min_edge_dist := minf(minf(dist_to_left, dist_to_right), minf(dist_to_top, dist_to_bottom))

	# Proactive edge avoidance - penalize before hitting the wall
	if min_edge_dist < GOAPConfigClass.POS_EDGE_WARNING_DIST:
		# Graduated penalty: stronger as we get closer to edge
		var edge_proximity := 1.0 - (min_edge_dist / GOAPConfigClass.POS_EDGE_WARNING_DIST)
		score -= edge_proximity * wall_avoidance * 60.0

		# Extra penalty for danger zone
		if min_edge_dist < GOAPConfigClass.POS_EDGE_DANGER_DIST:
			score -= wall_avoidance * 50.0

	# --- WALL PENALTY (existing but boosted) ---
	var wall_flags := ArenaUtilityClass.get_wall_flags(pos, 80.0)  # Larger detection range
	if wall_flags != ArenaUtilityClass.WallFlags.NONE:
		var wall_count := 0
		if wall_flags & ArenaUtilityClass.WallFlags.LEFT: wall_count += 1
		if wall_flags & ArenaUtilityClass.WallFlags.RIGHT: wall_count += 1
		if wall_flags & ArenaUtilityClass.WallFlags.TOP: wall_count += 1
		if wall_flags & ArenaUtilityClass.WallFlags.BOTTOM: wall_count += 1

		score -= wall_count * wall_avoidance * 50.0  # Increased from 40

	# Corner penalty (extra harsh)
	if ArenaUtilityClass.is_in_corner(pos, 120.0):  # Larger corner detection
		score -= wall_avoidance * 120.0  # Much harsher corner penalty

	# --- CORNERED ESCAPE BONUS ---
	# If we're currently in a corner or near edge, heavily reward moving toward center
	var current_in_corner := ArenaUtilityClass.is_in_corner(from_pos, 150.0)
	var current_near_edge := ArenaUtilityClass.get_wall_flags(from_pos, GOAPConfigClass.POS_EDGE_WARNING_DIST) != ArenaUtilityClass.WallFlags.NONE

	if current_in_corner or current_near_edge:
		var dist_to_center := pos.distance_to(center)
		var current_dist_to_center := from_pos.distance_to(center)
		if dist_to_center < current_dist_to_center:
			var escape_bonus := 150.0 if current_in_corner else 80.0
			score += escape_bonus  # Strong bonus for moving toward center

	return score


## Gets the best position from candidates
## Returns the highest-scoring valid position
func get_best_position(from_pos: Vector2, context: Dictionary) -> Vector2:
	# Add from_pos to context for scoring
	var full_context := context.duplicate()
	full_context["from_pos"] = from_pos

	var candidates := generate_candidates(from_pos, full_context)

	if candidates.is_empty():
		# No valid candidates, return current position
		return from_pos

	var best_pos := candidates[0]
	var best_score := score_position(best_pos, full_context)

	for i in range(1, candidates.size()):
		var candidate := candidates[i]
		var candidate_score := score_position(candidate, full_context)

		if candidate_score > best_score:
			best_score = candidate_score
			best_pos = candidate

	return best_pos


## Applies mode-specific weight multipliers
## Call this before get_best_position to adjust weights for different behaviors
func apply_mode_multipliers(mode: String) -> void:
	match mode:
		"retreat":
			weight_threat_distance *= retreat_threat_mult
			weight_strafe_preference *= 0.8
		"kite":
			weight_threat_distance *= 1.5
			weight_strafe_preference *= kite_strafe_mult
			weight_ammo_proximity *= 1.5
		"approach":
			weight_threat_distance *= -1.0  # Invert - want to get closer
			weight_strafe_preference *= 0.3
		"seek_ammo":
			weight_ammo_proximity *= 3.0
			weight_threat_distance *= 0.5
		"seek_health":
			weight_health_proximity *= 3.0
			weight_threat_distance *= 0.5
		"seek_cover":
			weight_cover_proximity *= 3.0
		"flank":
			weight_strafe_preference *= 2.0
			weight_threat_distance *= 0.5
		"evade":
			weight_threat_distance *= 3.0
			weight_center_proximity *= 0.5


## Resets weights to their base values (call after apply_mode_multipliers)
func reset_to_base_weights(base_weights: Dictionary) -> void:
	weight_threat_distance = base_weights.get("threat", 1.0)
	weight_ammo_proximity = base_weights.get("ammo", 0.5)
	weight_health_proximity = base_weights.get("health", 0.3)
	weight_cover_proximity = base_weights.get("cover", 0.4)
	weight_center_proximity = base_weights.get("center", 0.2)
	weight_strafe_preference = base_weights.get("strafe", 0.6)
	weight_los_to_target = base_weights.get("los", 0.3)
	weight_speed_boost_proximity = base_weights.get("speed_boost", 0.2)


## Builds a context dictionary from blackboard and agent state
## Helper for tasks that use this evaluator
static func build_context_from_blackboard(blackboard: Blackboard, agent: Node2D) -> Dictionary:
	var context := {}

	# Threat position and velocity (for prediction)
	var target: Node2D = blackboard.get_var(&"target", null)
	if is_instance_valid(target):
		context["threat_pos"] = target.global_position
		# Get threat velocity for predictive pathing
		if "velocity" in target:
			context["threat_velocity"] = target.velocity
		else:
			context["threat_velocity"] = Vector2.ZERO

	# Resource positions
	var ammo_pickup: Node2D = blackboard.get_var(&"ammo_pickup", null)
	if is_instance_valid(ammo_pickup) and (not ammo_pickup.has_method("is_available") or ammo_pickup.is_available()):
		context["ammo_pos"] = ammo_pickup.global_position

	var health_pickup: Node2D = blackboard.get_var(&"health_pickup", null)
	if is_instance_valid(health_pickup) and (not health_pickup.has_method("is_available") or health_pickup.is_available()):
		context["health_pos"] = health_pickup.global_position

	var cover_object: Node2D = blackboard.get_var(&"cover_object", null)
	if is_instance_valid(cover_object):
		context["cover_pos"] = cover_object.global_position

	var speed_boost_pickup: Node2D = blackboard.get_var(&"speed_boost_pickup", null)
	if is_instance_valid(speed_boost_pickup) and (not speed_boost_pickup.has_method("is_available") or speed_boost_pickup.is_available()):
		context["speed_boost_pos"] = speed_boost_pickup.global_position

	# Agent state ratios
	if "health" in agent and "max_health" in agent:
		context["health_ratio"] = float(agent.health) / float(agent.max_health)
	else:
		context["health_ratio"] = 1.0

	if "ammo_count" in agent and "max_ammo" in agent:
		context["ammo_ratio"] = float(agent.ammo_count) / float(agent.max_ammo)
	else:
		context["ammo_ratio"] = 1.0

	# Speed boost state
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if "is_speed_boosted" in combat:
			context["has_speed_boost"] = combat.is_speed_boosted

	# Weapon type for weapon-aware positioning
	context["is_ranged_weapon"] = _detect_ranged_weapon(agent)

	return context


## Detects if the agent currently has a ranged weapon equipped
static func _detect_ranged_weapon(agent: Node2D) -> bool:
	# Check for equipped_weapon property
	if "equipped_weapon" in agent:
		var weapon = agent.equipped_weapon
		if weapon != null:
			# Check for weapon_type property
			if "weapon_type" in weapon:
				return weapon.weapon_type == "ranged"
			# Fallback: check for has_ammo indicator
			if "uses_ammo" in weapon:
				return weapon.uses_ammo

	# Check via blackboard
	if agent.has_node("Blackboard"):
		var blackboard = agent.get_node("Blackboard")
		var has_ranged: bool = blackboard.get_var(&"has_ranged_weapon", false)
		return has_ranged

	# Fallback: check if agent has ammo (implies ranged weapon)
	if "ammo_count" in agent and "max_ammo" in agent:
		return agent.max_ammo > 0 and agent.ammo_count > 0

	return false
