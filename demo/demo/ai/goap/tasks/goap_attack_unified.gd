@tool
extends BTAction
## Unified attack action that adapts behavior based on weapon type.
## For RANGED: Throws ninja stars with strafing movement.
## For MELEE: Performs close-range melee attack.
## Returns SUCCESS after attack completes or kills target.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Damage dealt by attack (base for both melee and ranged)
@export var damage := 35

## Cooldown between ranged shots in seconds
@export var ranged_cooldown := 0.25

## Strafe speed while attacking (ranged only)
@export var strafe_speed := 180.0

## How often to change strafe direction (seconds)
@export var strafe_change_interval := 0.8

## Ninja star scene to spawn (ranged attacks)
const NINJA_STAR_SCENE = preload("res://demo/agents/ninja_star/ninja_star.tscn")

var _cooldown_timer := 0.0
var _strafe_direction := 1  # 1 = right, -1 = left
var _strafe_timer := 0.0
var _melee_cooldown_timer := 0.0  # Cooldown between melee swings
var _melee_swing_active := false   # True during melee swing animation


func _generate_name() -> String:
	return "GOAPAttack  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_cooldown_timer = 0.0
	_strafe_direction = 1 if randf() > 0.5 else -1
	_strafe_timer = randf() * strafe_change_interval
	# Don't reset melee cooldown on enter - maintain between attacks
	_melee_swing_active = false


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	# Get weapon type from combat component
	var weapon_type: int = CombatComponentClass.WeaponType.NONE
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		weapon_type = combat.weapon_type

	# No weapon equipped
	if weapon_type == CombatComponentClass.WeaponType.NONE:
		print("GOAP: Attack failed - no weapon equipped!")
		return FAILURE

	# Route to appropriate attack type
	if weapon_type == CombatComponentClass.WeaponType.MELEE:
		return _melee_attack(target_node, delta)
	else:
		return _ranged_attack(target_node, delta)


## Performs a melee attack on the target (with cooldown and movement slow)
func _melee_attack(target_node: Node2D, delta: float) -> Status:
	# Update agent facing toward target
	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()
	_update_facing(dir_to_target)

	# Apply movement slow during swing (can still move, just slower)
	if _melee_swing_active:
		var base_speed := GOAPConfigClass.MELEE_MOVE_SPEED
		var slow_speed := base_speed * GOAPConfigClass.MELEE_SWING_SLOW_FACTOR
		agent.velocity = dir_to_target * slow_speed  # Slowly advance during swing

	# Wait for cooldown between swings
	if _melee_cooldown_timer > 0.0:
		_melee_cooldown_timer -= delta
		return RUNNING

	# Ready to swing!
	_melee_swing_active = true
	_melee_cooldown_timer = GOAPConfigClass.MELEE_SWING_COOLDOWN

	# Play melee swing animation
	if agent.has_method("play_melee_swing"):
		agent.play_melee_swing()

	# Deal damage to target
	if target_node.has_node("Health"):
		var health = target_node.get_node("Health")
		if health.has_method("take_damage"):
			health.take_damage(damage, Vector2.ZERO)
			print("GOAP: Melee attack dealt %d damage!" % damage)
			_melee_swing_active = false
			return SUCCESS

	_melee_swing_active = false
	return FAILURE


## Performs a ranged attack (throws ninja star)
func _ranged_attack(target_node: Node2D, delta: float) -> Status:
	# Can't shoot while in cover
	var is_in_cover: bool = blackboard.get_var(&"in_cover", false)
	if is_in_cover:
		print("GOAP: Can't shoot while in cover!")
		return FAILURE

	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()

	# Apply strafing movement
	_apply_strafe_movement(delta, dir_to_target)

	# Wait for cooldown
	if _cooldown_timer > 0.0:
		_cooldown_timer -= delta
		return RUNNING

	# Consume ammo
	if agent.has_method("use_ammo"):
		if not agent.use_ammo():
			print("GOAP: No ammo for ranged attack!")
			return FAILURE

	# Reset cooldown
	_cooldown_timer = ranged_cooldown

	# Update facing
	_update_facing(dir_to_target)

	# Spawn ninja star
	var star = NINJA_STAR_SCENE.instantiate()
	var spawn_offset := dir_to_target * 60.0
	star.global_position = agent.global_position + spawn_offset + Vector2(0, -40)
	star.direction = dir_to_target
	star.shooter = agent
	agent.get_parent().add_child(star)

	# Apply suppression to target
	_apply_suppression_to_target(target_node)

	print("GOAP: Threw ninja star!")
	return RUNNING


## Updates agent facing direction
func _update_facing(dir_to_target: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir_to_target.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0


## Applies suppression effect to the target
func _apply_suppression_to_target(target_node: Node2D) -> void:
	if target_node.has_node("CombatComponent"):
		var target_combat = target_node.get_node("CombatComponent")
		if target_combat.has_method("apply_suppression"):
			target_combat.apply_suppression()


## Applies strafing movement perpendicular to the target (ranged only)
## When under melee threat, adds backward component for diagonal retreat while shooting
func _apply_strafe_movement(delta: float, dir_to_target: Vector2) -> void:
	_strafe_timer += delta
	if _strafe_timer >= strafe_change_interval:
		_strafe_timer = 0.0
		if randf() < 0.7:
			_strafe_direction *= -1

	# Base perpendicular strafe
	var strafe_dir: Vector2 = Vector2(-dir_to_target.y, dir_to_target.x) * _strafe_direction

	# If under melee threat, add backward component (diagonal retreat while shooting)
	var melee_threat: bool = blackboard.get_var(&"melee_threat", false)
	if melee_threat:
		var backward_dir: Vector2 = -dir_to_target
		# Blend: 60% backward, 40% strafe = diagonal retreat
		strafe_dir = (backward_dir * 0.6 + strafe_dir * 0.4).normalized()

	# Get speed from combat component (ranged is faster)
	var move_speed := strafe_speed
	if agent.has_node("CombatComponent"):
		var combat = agent.get_node("CombatComponent")
		if combat.has_method("get_move_speed"):
			move_speed = combat.get_move_speed()

	# Check bounds
	var next_pos: Vector2 = agent.global_position + strafe_dir * move_speed * delta
	if next_pos.x < GOAPConfigClass.ARENA_MIN.x or next_pos.x > GOAPConfigClass.ARENA_MAX.x:
		_strafe_direction *= -1
		strafe_dir.x *= -1
		strafe_dir = strafe_dir.normalized()
	if next_pos.y < GOAPConfigClass.ARENA_MIN.y or next_pos.y > GOAPConfigClass.ARENA_MAX.y:
		_strafe_direction *= -1
		strafe_dir.y *= -1
		strafe_dir = strafe_dir.normalized()

	agent.velocity = strafe_dir * move_speed
	agent.animation_player.play(&"walk")
