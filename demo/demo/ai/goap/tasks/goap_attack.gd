@tool
extends BTAction
## Throws a ninja star at the target as a ranged attack.
## Strafes while shooting for dynamic combat movement.
## Also triggers suppression on the target.
## Returns SUCCESS after firing, with a cooldown between shots.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Cooldown between shots in seconds
@export var cooldown := 0.25

## Strafe speed while attacking
@export var strafe_speed := 180.0

## How often to change strafe direction (seconds)
@export var strafe_change_interval := 0.8

## Ninja star scene to spawn
const NINJA_STAR_SCENE = preload("res://demo/agents/ninja_star/ninja_star.tscn")

var _cooldown_timer := 0.0
var _strafe_direction := 1  # 1 = right, -1 = left
var _strafe_timer := 0.0


func _generate_name() -> String:
	return "GOAPShoot  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_cooldown_timer = 0.0
	_strafe_direction = 1 if randf() > 0.5 else -1
	_strafe_timer = randf() * strafe_change_interval  # Random initial offset


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	# Can't shoot while in cover - abort the action
	var is_in_cover: bool = blackboard.get_var(&"in_cover", false)
	if is_in_cover:
		print("GOAP: Can't shoot while in cover!")
		return FAILURE

	# Calculate direction to target for strafing
	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()

	# Apply strafing movement (perpendicular to target direction)
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

	# Reset cooldown for next shot
	_cooldown_timer = cooldown

	# Update agent facing
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if dir_to_target.x > 0:
			root.scale.x = scale_magnitude
		else:
			root.scale.x = -scale_magnitude

	# Spawn ninja star offset from agent to avoid self-collision
	var star = NINJA_STAR_SCENE.instantiate()
	# Offset 60 pixels in direction of target to clear shooter's hurtbox
	var spawn_offset := dir_to_target * 60.0
	star.global_position = agent.global_position + spawn_offset + Vector2(0, -40)  # Also offset Y to match body height
	star.direction = dir_to_target
	star.shooter = agent  # Set shooter to prevent self-damage
	agent.get_parent().add_child(star)

	# Apply suppression to target (even if projectile might miss later)
	_apply_suppression_to_target(target_node)

	print("GOAP: Threw ninja star!")
	return RUNNING


## Applies suppression effect to the target
func _apply_suppression_to_target(target_node: Node2D) -> void:
	if target_node.has_node("CombatComponent"):
		var target_combat = target_node.get_node("CombatComponent")
		if target_combat.has_method("apply_suppression"):
			target_combat.apply_suppression()


## Applies strafing movement perpendicular to the target
func _apply_strafe_movement(delta: float, dir_to_target: Vector2) -> void:
	# Update strafe timer and potentially change direction
	_strafe_timer += delta
	if _strafe_timer >= strafe_change_interval:
		_strafe_timer = 0.0
		# 70% chance to change direction for unpredictability
		if randf() < 0.7:
			_strafe_direction *= -1

	# Calculate perpendicular strafe direction
	var strafe_dir: Vector2 = Vector2(-dir_to_target.y, dir_to_target.x) * _strafe_direction

	# Check if strafe would take us out of bounds, reverse if so
	var next_pos: Vector2 = agent.global_position + strafe_dir * strafe_speed * delta
	if next_pos.x < GOAPConfigClass.ARENA_MIN.x or next_pos.x > GOAPConfigClass.ARENA_MAX.x:
		_strafe_direction *= -1
		strafe_dir = Vector2(-dir_to_target.y, dir_to_target.x) * _strafe_direction
	if next_pos.y < GOAPConfigClass.ARENA_MIN.y or next_pos.y > GOAPConfigClass.ARENA_MAX.y:
		_strafe_direction *= -1
		strafe_dir = Vector2(-dir_to_target.y, dir_to_target.x) * _strafe_direction

	# Apply velocity for strafing
	agent.velocity = strafe_dir * strafe_speed
	agent.animation_player.play(&"walk")
