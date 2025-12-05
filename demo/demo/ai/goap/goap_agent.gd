## GOAP Demo Agent
## A tactical agent that uses GOAP to plan and execute decisions
## Delegates responsibilities to child components for cleaner architecture
## Supports agent-vs-agent combat with multiple cover objects and team battles
extends CharacterBody2D

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")
const MovementComponentClass = preload("res://demo/ai/goap/components/movement_component.gd")
const WorldStateManagerClass = preload("res://demo/ai/goap/components/world_state_manager.gd")
const GoalEvaluatorClass = preload("res://demo/ai/goap/components/goal_evaluator.gd")

signal target_killed
signal health_changed(current: int, max_health: int)
signal ammo_changed(current: int)
signal died

# Agent identification
@export var agent_name: String = "Agent"
@export var team: int = 0  # 0 = Red team, 1 = Blue team

# World objects (set via inspector paths, resolved at runtime)
@export_node_path("Node2D") var target_path: NodePath
@export_node_path("Node2D") var weapon_pickup_path: NodePath
@export_node_path("Node2D") var ammo_pickup_path: NodePath
@export_node_path("Node2D") var health_pickup_path: NodePath
@export_node_path("Node2D") var cover_object_path: NodePath  # Single cover for basic demos

# Resolved node references
var target: Node2D
var weapon_pickup: Node2D
var ammo_pickup: Node2D
var health_pickup: Node2D

# Multiple enemies support for team battles
var enemies: Array[Node2D] = []

# Multiple cover objects support
var cover_objects: Array[Node2D] = []

# GOAP Goals for dynamic switching
@export var goal_kill_target: Resource  # GOAPGoal
@export var goal_avoid_damage: Resource  # GOAPGoal
@export var goal_regain_health: Resource  # GOAPGoal

# Cover state (managed by actions)
var in_cover := false

# Current best cover (updated by WorldStateManager)
var cover_object: Node2D:
	get:
		if world_state:
			return world_state.current_cover
		return null

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
	# Resolve exported NodePaths to actual node references
	if target_path:
		target = get_node_or_null(target_path)
	if weapon_pickup_path:
		weapon_pickup = get_node_or_null(weapon_pickup_path)
	if ammo_pickup_path:
		ammo_pickup = get_node_or_null(ammo_pickup_path)
	if health_pickup_path:
		health_pickup = get_node_or_null(health_pickup_path)
	if cover_object_path:
		var cover = get_node_or_null(cover_object_path)
		if cover:
			cover_objects.append(cover)


func _setup_components() -> void:
	# Configure WorldStateManager
	if world_state:
		world_state.bt_player = bt_player
		world_state.agent = self
		world_state.target = target
		world_state.weapon_pickup = weapon_pickup
		world_state.ammo_pickup = ammo_pickup
		world_state.health_pickup = health_pickup
		world_state.set_cover_objects(cover_objects)

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
	# Connect enemy attack signal for immediate threat response (legacy support)
	if target and target.has_signal("attack_started"):
		target.attack_started.connect(_on_threat_detected)

	# Connect combat component signals
	if combat:
		combat.health_changed.connect(_on_combat_health_changed)
		combat.ammo_changed.connect(_on_combat_ammo_changed)
		combat.died.connect(_on_combat_died)

	# Connect world state signals for goal evaluation
	if world_state:
		world_state.threat_changed.connect(_on_threat_state_changed)
		world_state.health_state_changed.connect(_on_health_state_changed)

	# Connect Health node signals (for hurtbox damage detection)
	if has_node("Health"):
		var health_node = get_node("Health")
		health_node.damaged.connect(_on_health_node_damaged)
		health_node.death.connect(_on_health_node_death)


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
	# Keep agent inside arena bounds
	global_position.x = clampf(global_position.x, GOAPConfigClass.ARENA_MIN.x, GOAPConfigClass.ARENA_MAX.x)
	global_position.y = clampf(global_position.y, GOAPConfigClass.ARENA_MIN.y, GOAPConfigClass.ARENA_MAX.y)
	# Update target to closest enemy (for team battles)
	_update_team_targeting()


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
	_set_crouch_visual(true)
	print("%s: Entered cover" % agent_name)


func leave_cover() -> void:
	in_cover = false
	_set_crouch_visual(false)
	print("%s: Left cover" % agent_name)


func _set_crouch_visual(crouching: bool) -> void:
	# Crouch by lowering the body sprite and scaling it down slightly
	if root and root.has_node("Rig/Body"):
		var body: Sprite2D = root.get_node("Rig/Body")
		var tween := create_tween()
		if crouching:
			# Duck down: lower position and squash slightly
			tween.tween_property(body, "position", Vector2(0, -20), 0.15)
			tween.parallel().tween_property(body, "scale", Vector2(1.0, 0.7), 0.15)
		else:
			# Stand up: restore position and scale
			tween.tween_property(body, "position", Vector2(0, -40), 0.15)
			tween.parallel().tween_property(body, "scale", Vector2(1.0, 1.0), 0.15)


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
	var current_cover := cover_object
	if not current_cover or not is_instance_valid(current_cover):
		return global_position

	if current_cover.has_method("get_cover_position_against"):
		if target and is_instance_valid(target):
			return current_cover.get_cover_position_against(target.global_position)

	return current_cover.global_position


## Sets the target dynamically (for agent-vs-agent combat)
func set_target(new_target: Node2D) -> void:
	target = new_target
	if world_state:
		world_state.target = new_target


## Adds a cover object to the list
func add_cover_object(cover: Node2D) -> void:
	if cover and not cover_objects.has(cover):
		cover_objects.append(cover)
		if world_state:
			world_state.set_cover_objects(cover_objects)


## Sets all cover objects at once
func set_cover_objects_array(covers: Array) -> void:
	cover_objects.clear()
	for cover in covers:
		if cover is Node2D:
			cover_objects.append(cover)
	if world_state:
		world_state.set_cover_objects(cover_objects)


## Sets all enemy agents for team battles
func set_enemies(enemy_list: Array) -> void:
	enemies.clear()
	for enemy in enemy_list:
		if enemy is Node2D and enemy != self:
			enemies.append(enemy)
	if world_state:
		world_state.enemies = enemies
	# Set initial target to closest enemy
	_update_target_to_closest_enemy()


## Updates target to the closest living enemy
func _update_target_to_closest_enemy() -> void:
	var closest: Node2D = null
	var closest_dist := INF

	for enemy in enemies:
		if not is_instance_valid(enemy):
			continue
		# Check if enemy is still alive
		if "health" in enemy and enemy.health <= 0:
			continue
		var dist := global_position.distance_to(enemy.global_position)
		if dist < closest_dist:
			closest_dist = dist
			closest = enemy

	if closest and closest != target:
		set_target(closest)


## Called every physics frame to update target selection
func _update_team_targeting() -> void:
	if enemies.size() > 0:
		_update_target_to_closest_enemy()


# Signal handlers
func _on_threat_detected() -> void:
	print("%s: Threat detected!" % agent_name)


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


func _on_combat_died() -> void:
	print("%s: DIED!" % agent_name)
	died.emit()


# Health node signal handlers (for hurtbox damage from projectiles)
func _on_health_node_damaged(amount: float, _knockback: Vector2) -> void:
	# Forward damage to combat component
	print("%s: HIT by projectile for %.0f damage!" % [agent_name, amount])
	take_damage(int(amount))


func _on_health_node_death() -> void:
	# Forward death to combat component (will emit died signal)
	print("%s: KILLED by projectile!" % agent_name)
	if combat:
		combat.health = 0
		combat._emit_health_changed()
	died.emit()
