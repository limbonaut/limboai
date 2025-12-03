## GOAP Demo Agent
## A tactical agent that uses GOAP to plan and execute decisions
## Delegates responsibilities to child components for cleaner architecture
extends CharacterBody2D

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const MovementComponentClass = preload("res://demo/ai/goap/components/movement_component.gd")
const WorldStateManagerClass = preload("res://demo/ai/goap/components/world_state_manager.gd")
const GoalEvaluatorClass = preload("res://demo/ai/goap/components/goal_evaluator.gd")

signal target_killed
signal health_changed(current: int, max_health: int)
signal ammo_changed(current: int)

# World objects (set via inspector or found at runtime)
@export_node_path("Node2D") var target_path: NodePath
@export_node_path("Node2D") var weapon_pickup_path: NodePath
@export_node_path("Node2D") var ammo_pickup_path: NodePath
@export_node_path("Node2D") var health_pickup_path: NodePath
@export_node_path("Node2D") var cover_object_path: NodePath

# GOAP Goals for dynamic switching
@export var goal_kill_target: Resource  # GOAPGoal
@export var goal_avoid_damage: Resource  # GOAPGoal
@export var goal_regain_health: Resource  # GOAPGoal

# World object references
var target: Node2D
var weapon_pickup: Node2D
var ammo_pickup: Node2D
var health_pickup: Node2D
var cover_object: Node2D

# Cover state (managed by actions)
var in_cover := false

# Component references
@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var root: Node2D = $Root
@onready var bt_player: BTPlayer = $BTPlayer
@onready var combat = $CombatComponent
@onready var movement = $MovementComponent
@onready var world_state = $WorldStateManager
@onready var goal_evaluator = $GoalEvaluator

# Proxy properties for backward compatibility with action tasks
var has_weapon: bool:
	get: return combat.has_weapon if combat else false
	set(value):
		if combat:
			combat.has_weapon = value

var ammo_count: int:
	get: return combat.ammo_count if combat else 0
	set(value):
		if combat:
			combat.ammo_count = value

var max_ammo: int:
	get: return combat.max_ammo if combat else GOAPConfigClass.DEFAULT_MAX_AMMO
	set(value):
		if combat:
			combat.max_ammo = value

var health: int:
	get: return combat.health if combat else 100
	set(value):
		if combat:
			combat.health = value

var max_health: int:
	get: return combat.max_health if combat else GOAPConfigClass.DEFAULT_MAX_HEALTH
	set(value):
		if combat:
			combat.max_health = value

var weapon_jammed: bool:
	get: return combat.weapon_jammed_state if combat else false

var jam_chance: float:
	get: return combat.jam_chance if combat else GOAPConfigClass.DEFAULT_JAM_CHANCE
	set(value):
		if combat:
			combat.jam_chance = value


func _ready() -> void:
	_resolve_node_paths()
	_setup_components()
	_connect_signals()
	_initial_sync()


func _resolve_node_paths() -> void:
	if not target_path.is_empty():
		target = get_node(target_path) as Node2D
	if not weapon_pickup_path.is_empty():
		weapon_pickup = get_node(weapon_pickup_path) as Node2D
	if not ammo_pickup_path.is_empty():
		ammo_pickup = get_node(ammo_pickup_path) as Node2D
	if not health_pickup_path.is_empty():
		health_pickup = get_node(health_pickup_path) as Node2D
	if not cover_object_path.is_empty():
		cover_object = get_node(cover_object_path) as Node2D


func _setup_components() -> void:
	# Configure WorldStateManager
	if world_state:
		world_state.bt_player = bt_player
		world_state.agent = self
		world_state.target = target
		world_state.weapon_pickup = weapon_pickup
		world_state.ammo_pickup = ammo_pickup
		world_state.health_pickup = health_pickup
		world_state.cover_object = cover_object

	# Configure MovementComponent
	if movement:
		movement.agent = self
		movement.root = root

	# Configure GoalEvaluator
	if goal_evaluator:
		goal_evaluator.bt_player = bt_player
		goal_evaluator.goal_kill_target = goal_kill_target
		goal_evaluator.goal_avoid_damage = goal_avoid_damage
		goal_evaluator.goal_regain_health = goal_regain_health


func _connect_signals() -> void:
	# Connect enemy attack signal for immediate threat response
	if target and target.has_signal("attack_started"):
		target.attack_started.connect(_on_threat_detected)

	# Connect combat component signals
	if combat:
		combat.health_changed.connect(_on_combat_health_changed)
		combat.ammo_changed.connect(_on_combat_ammo_changed)

	# Connect world state signals for goal evaluation
	if world_state:
		world_state.threat_changed.connect(_on_threat_state_changed)
		world_state.health_state_changed.connect(_on_health_state_changed)


func _initial_sync() -> void:
	var bb := bt_player.get_blackboard()
	if bb:
		bb.set_var(&"under_threat", false)
		bb.set_var(&"enemy_attacking", false)
		bb.set_var(&"in_cover", false)
		bb.set_var(&"near_cover", false)
		bb.set_var(&"weapon_jammed", false)
		bb.set_var(&"low_health", false)

	if world_state:
		world_state.force_sync()

	health_changed.emit(health, max_health)
	ammo_changed.emit(ammo_count)


func _physics_process(_delta: float) -> void:
	move_and_slide()


# Combat API (delegates to component)
func use_ammo() -> bool:
	return combat.use_ammo() if combat else false


func unjam_weapon() -> void:
	if combat:
		combat.unjam_weapon()


func add_ammo(amount: int) -> void:
	if combat:
		combat.add_ammo(amount)


func take_damage(amount: int) -> void:
	if combat:
		combat.take_damage(amount)


func heal(amount: int) -> void:
	if combat:
		combat.heal(amount)


# Cover API
func enter_cover() -> void:
	in_cover = true
	print("GOAP: Entered cover")


func leave_cover() -> void:
	in_cover = false
	print("GOAP: Left cover")


# Movement API (delegates to component)
func move(p_velocity: Vector2) -> void:
	if movement:
		movement.move(p_velocity)
	else:
		velocity = lerp(velocity, p_velocity, 0.2)
		move_and_slide()
		update_facing()


func update_facing() -> void:
	if movement:
		movement.update_facing()
	elif root:
		if velocity.x > 10 and root.scale.x < 0:
			root.scale.x = 1.0
		elif velocity.x < -10 and root.scale.x > 0:
			root.scale.x = -1.0


func get_facing() -> float:
	return movement.get_facing() if movement else signf(root.scale.x)


# Line of sight check
func has_los_to_target() -> bool:
	if not target or not is_instance_valid(target):
		return false
	var space_state := get_world_2d().direct_space_state
	var query := PhysicsRayQueryParameters2D.create(
		global_position,
		target.global_position,
		GOAPConfigClass.LOS_COLLISION_LAYER
	)
	var result := space_state.intersect_ray(query)
	return result.is_empty()


## Returns the position the agent should move to for cover against the target
func get_cover_position() -> Vector2:
	if not cover_object or not is_instance_valid(cover_object):
		return global_position

	if cover_object.has_method("get_cover_position_against"):
		if target and is_instance_valid(target):
			return cover_object.get_cover_position_against(target.global_position)

	return cover_object.global_position


# Signal handlers
func _on_threat_detected() -> void:
	print("GOAP: Threat detected!")


func _on_threat_state_changed(is_threatened: bool) -> void:
	var is_low: bool = combat.is_low_health() if combat else false
	if goal_evaluator:
		goal_evaluator.evaluate(is_threatened, is_low)


func _on_health_state_changed(is_low_health: bool) -> void:
	var bb := bt_player.get_blackboard()
	var is_threatened := false
	if bb:
		is_threatened = bb.get_var(&"under_threat", false)
	if goal_evaluator:
		goal_evaluator.evaluate(is_threatened, is_low_health)


func _on_combat_health_changed(current: int, max_hp: int) -> void:
	health_changed.emit(current, max_hp)


func _on_combat_ammo_changed(current: int) -> void:
	ammo_changed.emit(current)
