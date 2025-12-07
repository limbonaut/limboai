@tool
extends BTAction
## Shoots at target while staying in cover (cover peeking).
## Has reduced accuracy (50%) compared to standing shots.
## Also triggers suppression on the target.
## Returns RUNNING while shooting, SUCCESS when out of ammo.

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const CombatComponentClass = preload("res://demo/ai/goap/components/combat_component.gd")

## Blackboard variable storing the target node
@export var target_var := &"target"

## Cooldown between shots in seconds (longer than normal)
@export var cooldown := 0.5

## Ninja star scene to spawn
const NINJA_STAR_SCENE = preload("res://demo/agents/ninja_star/ninja_star.tscn")

var _cooldown_timer := 0.0


func _generate_name() -> String:
	return "GOAPShootFromCover  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	_cooldown_timer = 0.0


func _tick(delta: float) -> Status:
	var target_node: Node2D = blackboard.get_var(target_var)
	if not is_instance_valid(target_node):
		return FAILURE

	# Must be in cover to use this action
	var is_in_cover: bool = blackboard.get_var(&"in_cover", false)
	if not is_in_cover:
		print("GOAP: ShootFromCover requires being in cover!")
		return FAILURE

	# Wait for cooldown
	if _cooldown_timer > 0.0:
		_cooldown_timer -= delta
		return RUNNING

	# Calculate direction to target
	var dir_to_target: Vector2 = (target_node.global_position - agent.global_position).normalized()

	# Consume ammo
	if agent.has_method("use_ammo"):
		if not agent.use_ammo():
			print("GOAP: No ammo for cover shot!")
			return FAILURE

	# Reset cooldown
	_cooldown_timer = cooldown

	# Update agent facing
	if agent.has_node("Root"):
		var root: Node2D = agent.get_node("Root")
		var scale_magnitude := absf(root.scale.x)
		if dir_to_target.x > 0:
			root.scale.x = scale_magnitude
		else:
			root.scale.x = -scale_magnitude

	# Apply suppression to target (always - even if we miss)
	_apply_suppression_to_target(target_node)

	# Check if this shot hits based on cover accuracy penalty
	var will_hit := true
	if agent.has_node("CombatComponent"):
		var combat: CombatComponentClass = agent.get_node("CombatComponent")
		# Temporarily apply cover accuracy penalty
		var original_accuracy := combat.accuracy_modifier
		combat.accuracy_modifier = GOAPConfigClass.COVER_ACCURACY_PENALTY
		will_hit = combat.should_hit()
		combat.accuracy_modifier = original_accuracy  # Restore

	if will_hit:
		# Spawn ninja star
		var star = NINJA_STAR_SCENE.instantiate()
		var spawn_offset := dir_to_target * 60.0
		star.global_position = agent.global_position + spawn_offset + Vector2(0, -40)
		star.direction = dir_to_target
		star.shooter = agent
		agent.get_parent().add_child(star)
		print("GOAP: Cover shot HIT!")
	else:
		print("GOAP: Cover shot MISSED (50%% accuracy penalty)")

	return RUNNING


## Applies suppression effect to the target
func _apply_suppression_to_target(target_node: Node2D) -> void:
	if target_node.has_node("CombatComponent"):
		var target_combat = target_node.get_node("CombatComponent")
		if target_combat.has_method("apply_suppression"):
			target_combat.apply_suppression()
