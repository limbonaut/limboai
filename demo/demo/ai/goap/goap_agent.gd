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

# Dodge impulse state
var _dodge_velocity := Vector2.ZERO
var _dodge_timer := 0.0
const DODGE_SPEED := 400.0
const DODGE_DURATION := 0.2
const DODGE_COOLDOWN := 0.5
var _dodge_cooldown_timer := 0.0

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
@export var goal_avoid_damage: Resource  # GOAPGoal (for ranged threats - seek cover)
@export var goal_regain_health: Resource  # GOAPGoal
@export var goal_evade_melee: Resource  # GOAPGoal (for melee threats - maintain distance)

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

# Weapon sprite (shown when weapon equipped)
var weapon_sprite: Sprite2D

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
	_find_weapon_sprite()
	_setup_components()
	_connect_signals()
	_initial_sync()


## Finds the weapon sprite node in the rig (if it exists)
func _find_weapon_sprite() -> void:
	if root and root.has_node("Rig/WeaponSprite"):
		weapon_sprite = root.get_node("Rig/WeaponSprite")
		weapon_sprite.visible = false  # Hidden until weapon is picked up


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
		goal_evaluator.goal_evade_melee = goal_evade_melee


func _connect_signals() -> void:
	# Connect enemy attack signal for immediate threat response (legacy support)
	if target and target.has_signal("attack_started"):
		target.attack_started.connect(_on_threat_detected)

	# Connect combat component signals
	if combat:
		combat.health_changed.connect(_on_combat_health_changed)
		combat.ammo_changed.connect(_on_combat_ammo_changed)
		combat.died.connect(_on_combat_died)
		combat.weapon_equipped.connect(_on_combat_weapon_equipped)

	# Connect world state signals for goal evaluation
	if world_state:
		world_state.threat_changed.connect(_on_threat_state_changed)
		world_state.health_state_changed.connect(_on_health_state_changed)
		# Connect new threat type signals for weapon-aware tactical decisions
		world_state.melee_threat_changed.connect(_on_melee_threat_changed)
		world_state.ranged_threat_changed.connect(_on_ranged_threat_changed)

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
		# Threat type variables for weapon-aware tactical decisions
		bb.set_var(&"melee_threat", false)
		bb.set_var(&"ranged_threat", false)
		bb.set_var(&"enemy_has_melee_weapon", false)
		bb.set_var(&"enemy_has_ranged_weapon", false)

	if world_state:
		world_state.force_sync()

	health_changed.emit(health, max_health)
	ammo_changed.emit(ammo_count)


func _physics_process(delta: float) -> void:
	# Update dodge cooldown
	if _dodge_cooldown_timer > 0.0:
		_dodge_cooldown_timer -= delta

	# Apply dodge impulse if active
	if _dodge_timer > 0.0:
		_dodge_timer -= delta
		velocity = _dodge_velocity * (_dodge_timer / DODGE_DURATION)  # Ease out
	# Lock movement while in cover (crouching)
	elif in_cover:
		velocity = Vector2.ZERO

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


func _on_melee_threat_changed(is_melee_threat: bool) -> void:
	if goal_evaluator:
		goal_evaluator.set_melee_threat(is_melee_threat)
		# Trigger re-evaluation with current threat/health state
		var bb: Blackboard = bt_player.get_blackboard()
		var is_threatened: bool = bool(bb.get_var(&"under_threat", false)) if bb else false
		var is_low: bool = combat.is_low_health() if combat else false
		goal_evaluator.evaluate(is_threatened, is_low)


func _on_ranged_threat_changed(is_ranged_threat: bool) -> void:
	if goal_evaluator:
		goal_evaluator.set_ranged_threat(is_ranged_threat)
		# Trigger re-evaluation with current threat/health state
		var bb: Blackboard = bt_player.get_blackboard()
		var is_threatened: bool = bool(bb.get_var(&"under_threat", false)) if bb else false
		var is_low: bool = combat.is_low_health() if combat else false
		goal_evaluator.evaluate(is_threatened, is_low)


func _on_combat_health_changed(current: int, max_hp: int) -> void:
	health_changed.emit(current, max_hp)


func _on_combat_ammo_changed(current: int) -> void:
	ammo_changed.emit(current)


func _on_combat_died() -> void:
	print("%s: DIED!" % agent_name)
	died.emit()


func _on_combat_weapon_equipped(_weapon_type: int) -> void:
	# Trigger immediate replan when weapon is equipped
	# This ensures agent plans with knowledge of actual weapon type
	print("%s: Weapon equipped, triggering replan" % agent_name)
	if goal_evaluator:
		goal_evaluator.force_replan()


# Health node signal handlers (for hurtbox damage from projectiles)
func _on_health_node_damaged(amount: float, _knockback: Vector2) -> void:
	# Forward damage to combat component
	print("%s: HIT by projectile for %.0f damage!" % [agent_name, amount])
	take_damage(int(amount))
	# Trigger dodge reaction if not on cooldown
	_trigger_dodge_reaction()


func _on_health_node_death() -> void:
	# Forward death to combat component (will emit died signal)
	print("%s: KILLED by projectile!" % agent_name)
	if combat:
		combat.health = 0
		combat._emit_health_changed()
	died.emit()


## Triggers a dodge reaction when hit - moves perpendicular to attacker
func _trigger_dodge_reaction() -> void:
	# Skip if on cooldown or in cover
	if _dodge_cooldown_timer > 0.0 or in_cover:
		return

	# Calculate dodge direction (perpendicular to attacker)
	var dodge_dir := Vector2.ZERO
	if target and is_instance_valid(target):
		var to_attacker := (target.global_position - global_position).normalized()
		# Choose random perpendicular direction
		if randf() > 0.5:
			dodge_dir = Vector2(-to_attacker.y, to_attacker.x)
		else:
			dodge_dir = Vector2(to_attacker.y, -to_attacker.x)
	else:
		# Random direction if no target
		dodge_dir = Vector2(randf_range(-1, 1), randf_range(-1, 1)).normalized()

	# Check bounds and reverse if needed
	var next_pos := global_position + dodge_dir * DODGE_SPEED * DODGE_DURATION
	if next_pos.x < GOAPConfigClass.ARENA_MIN.x or next_pos.x > GOAPConfigClass.ARENA_MAX.x:
		dodge_dir.x *= -1
	if next_pos.y < GOAPConfigClass.ARENA_MIN.y or next_pos.y > GOAPConfigClass.ARENA_MAX.y:
		dodge_dir.y *= -1

	# Apply dodge
	_dodge_velocity = dodge_dir * DODGE_SPEED
	_dodge_timer = DODGE_DURATION
	_dodge_cooldown_timer = DODGE_COOLDOWN
	print("%s: Dodge!" % agent_name)


## Updates the weapon sprite visual based on weapon type
func update_weapon_visual() -> void:
	if not weapon_sprite:
		return

	if not combat or not combat.has_weapon:
		weapon_sprite.visible = false
		return

	weapon_sprite.visible = true

	# Adjust visual based on weapon type
	if combat.is_melee():
		# Melee weapon: longer, sword-like shape
		weapon_sprite.rotation_degrees = 45.0
		weapon_sprite.scale = Vector2(3.0, 0.8)  # Elongated
		weapon_sprite.modulate = Color(1.0, 0.7, 0.3, 1.0)  # Orange/bronze tint
		weapon_sprite.position = Vector2(35, -50)  # Higher up like a sword
	else:
		# Ranged weapon: ninja star shape
		weapon_sprite.rotation_degrees = 0.0
		weapon_sprite.scale = Vector2(1.5, 1.5)
		weapon_sprite.modulate = Color(1.0, 1.0, 1.0, 1.0)  # White/silver
		weapon_sprite.position = Vector2(40, -40)


## Plays a melee swing animation using tween
func play_melee_swing() -> void:
	if not weapon_sprite or not weapon_sprite.visible:
		return

	# Animate swing: quick rotation forward then back
	var start_rotation: float = weapon_sprite.rotation_degrees
	var swing_rotation: float = start_rotation - 90.0  # Swing downward

	var tween := create_tween()
	tween.tween_property(weapon_sprite, "rotation_degrees", swing_rotation, 0.1)
	tween.tween_property(weapon_sprite, "rotation_degrees", start_rotation, 0.15)
