@tool
extends BTAction
## Kite Attack: Shoots while backing away from melee threats.
## Combines retreat movement with ranged shooting for optimal kiting behavior.
## Returns RUNNING while kiting, SUCCESS when safe distance reached or target dead.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Damage dealt by attack
@export var damage := 35

## Cooldown between shots in seconds
@export var shot_cooldown := 0.3

## Ninja star scene to spawn
const NINJA_STAR_SCENE = preload("res://demo/agents/ninja_star/ninja_star.tscn")

var _cooldown_timer := 0.0


func _generate_name() -> String:
	return "GOAPKiteAttack  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_cooldown_timer = 0.0
	print("GOAP: Starting kite attack!")


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	# Check if we have ranged weapon and ammo
	if not agent.has_node("CombatComponent"):
		return FAILURE
	var combat = agent.get_node("CombatComponent")
	if not combat.is_ranged():
		print("GOAP: Kite attack requires ranged weapon!")
		return FAILURE

	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()
	var distance: float = agent.global_position.distance_to(target_node.global_position)

	# Check if we've reached safe distance
	if distance >= GOAPConfigClass.RETREAT_DISTANCE:
		agent.velocity = Vector2.ZERO
		print("GOAP: Kite attack - safe distance reached!")
		return SUCCESS

	# Get movement speed (ranged is faster)
	var speed: float = combat.get_move_speed()

	# Move backward (away from target)
	var retreat_dir: Vector2 = -dir_to_target

	# Check arena bounds and adjust if needed
	var next_pos: Vector2 = agent.global_position + retreat_dir * speed * delta
	if next_pos.x < GOAPConfigClass.ARENA_MIN.x or next_pos.x > GOAPConfigClass.ARENA_MAX.x:
		retreat_dir.x = 0.0
		retreat_dir = retreat_dir.normalized() if retreat_dir.length() > 0 else Vector2.UP
	if next_pos.y < GOAPConfigClass.ARENA_MIN.y or next_pos.y > GOAPConfigClass.ARENA_MAX.y:
		retreat_dir.y = 0.0
		retreat_dir = retreat_dir.normalized() if retreat_dir.length() > 0 else Vector2.RIGHT

	# Apply retreat movement
	if retreat_dir.length() > 0.1:
		agent.velocity = retreat_dir * speed
	else:
		agent.velocity = Vector2.ZERO

	# Keep facing the target while retreating
	_update_facing(dir_to_target)

	# Play walk animation
	if "animation_player" in agent:
		agent.animation_player.play(&"walk")

	# Handle shooting while retreating
	_cooldown_timer -= delta
	if _cooldown_timer <= 0.0:
		if _shoot_at_target(target_node, dir_to_target):
			_cooldown_timer = shot_cooldown
		else:
			# Out of ammo or jammed - return success to let GOAP replan
			print("GOAP: Kite attack - out of ammo!")
			return SUCCESS

	return RUNNING


func _shoot_at_target(target_node: Node2D, dir_to_target: Vector2) -> bool:
	# Consume ammo
	if agent.has_method("use_ammo"):
		if not agent.use_ammo():
			return false

	# Spawn ninja star
	var star = NINJA_STAR_SCENE.instantiate()
	var spawn_offset := dir_to_target * 60.0
	star.global_position = agent.global_position + spawn_offset + Vector2(0, -40)
	star.direction = dir_to_target
	star.shooter = agent
	agent.get_parent().add_child(star)

	# Apply suppression to target
	if target_node.has_node("CombatComponent"):
		var target_combat = target_node.get_node("CombatComponent")
		if target_combat.has_method("apply_suppression"):
			target_combat.apply_suppression()

	print("GOAP: Kite attack - threw ninja star!")
	return true


## Updates agent facing direction
func _update_facing(dir_to_target: Vector2) -> void:
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		if dir_to_target.x > 0:
			root.scale.x = 1.0
		else:
			root.scale.x = -1.0
