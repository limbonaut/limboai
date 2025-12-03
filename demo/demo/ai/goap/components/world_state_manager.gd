## World State Manager
## Manages GOAP world state synchronization to blackboard
## Emits signals when key facts change for goal evaluation
class_name WorldStateManager
extends Node

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

signal threat_changed(is_threatened: bool)
signal health_state_changed(is_low_health: bool)
signal target_killed

@export var bt_player: BTPlayer
@export var agent: CharacterBody2D

# World references (set by parent)
var target: Node2D
var weapon_pickup: Node2D
var ammo_pickup: Node2D
var health_pickup: Node2D
var cover_object: Node2D

# Cached previous states for change detection
var _prev_under_threat := false
var _prev_low_health := false


func _physics_process(_delta: float) -> void:
	_sync_world_state()


func _sync_world_state() -> void:
	var bb := _get_blackboard()
	if not bb or not agent:
		return

	# Compute all facts
	var facts := _compute_facts()

	# Detect state changes BEFORE updating blackboard
	var under_threat: bool = facts.get("under_threat", false)
	var low_health: bool = facts.get("low_health", false)

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
	var weapon_loaded := has_weapon and has_ammo
	var low_health: bool = health < GOAPConfigClass.LOW_HEALTH_THRESHOLD
	var is_healthy: bool = health >= GOAPConfigClass.HEALTHY_THRESHOLD

	# Enemy attack state
	var enemy_attacking := false
	if target and is_instance_valid(target):
		if target.has_method("is_preparing_attack"):
			enemy_attacking = target.is_preparing_attack()

	var under_threat := enemy_attacking

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
	facts["enemy_attacking"] = enemy_attacking
	facts["under_threat"] = under_threat

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
	facts["cover_object"] = cover_object

	return facts


func _compute_proximity_facts() -> Dictionary:
	var facts := {}
	var agent_pos: Vector2 = agent.global_position

	# Weapon pickup
	var near_weapon_pickup := false
	var weapon_available := false
	if weapon_pickup and is_instance_valid(weapon_pickup):
		weapon_available = _is_pickup_available(weapon_pickup)
		if weapon_available:
			near_weapon_pickup = agent_pos.distance_to(weapon_pickup.global_position) < GOAPConfigClass.PICKUP_RANGE

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

	# Cover
	var near_cover := false
	if cover_object and is_instance_valid(cover_object):
		near_cover = agent_pos.distance_to(cover_object.global_position) < GOAPConfigClass.COVER_RANGE

	facts["near_weapon_pickup"] = near_weapon_pickup
	facts["weapon_available"] = weapon_available
	facts["near_ammo"] = near_ammo
	facts["ammo_available"] = ammo_available
	facts["near_health_pickup"] = near_health_pickup
	facts["health_available"] = health_available
	facts["near_cover"] = near_cover

	return facts


func _compute_target_facts() -> Dictionary:
	var facts := {}

	var target_in_range := false
	var target_in_sight := false
	var target_visible := false
	var target_dead := false

	if target == null or not is_instance_valid(target):
		target_dead = true
	else:
		var dist := agent.global_position.distance_to(target.global_position)
		target_in_range = dist < GOAPConfigClass.ATTACK_RANGE
		target_in_sight = dist < GOAPConfigClass.SHOOTING_RANGE
		target_visible = _has_los_to_target()

		# Check if target is dead
		if target.has_node("Health"):
			var health_node = target.get_node("Health")
			if health_node.has_method("get_current"):
				target_dead = health_node.get_current() <= 0

	facts["target_in_range"] = target_in_range
	facts["target_in_sight"] = target_in_sight
	facts["target_visible"] = target_visible
	facts["target_dead"] = target_dead

	return facts


func _is_pickup_available(pickup: Node2D) -> bool:
	if pickup.has_method("is_available"):
		return pickup.is_available()
	return true  # Fallback for pickups without availability check


func _has_los_to_target() -> bool:
	if not target or not is_instance_valid(target):
		return false

	var space_state := agent.get_world_2d().direct_space_state
	var query := PhysicsRayQueryParameters2D.create(
		agent.global_position,
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
