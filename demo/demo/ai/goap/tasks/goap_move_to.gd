@tool
extends BTAction
## Moves the GOAP agent toward a target node stored in the blackboard.
## Returns SUCCESS when close to target, RUNNING while moving.
## REACTIVE: Returns FAILURE if melee threat gets too close (for ranged agents).

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

## Blackboard variable that stores the target node (Node2D)
@export var target_var := &"target"

## How close should the agent be to return SUCCESS
@export var tolerance := 80.0

## Movement speed
@export var speed := 300.0

## Enable reactive threat abort (for ranged agents approaching targets)
@export var abort_on_melee_threat := true


func _generate_name() -> String:
	return "GOAPMoveTo  target: %s" % LimboUtility.decorate_var(target_var)


func _enter() -> void:
	print("GOAP ACTION: MoveTo %s started" % target_var)


func _tick(_delta: float) -> Status:
	# REACTIVE: Check if a melee threat is too close (for ranged agents)
	if abort_on_melee_threat and _should_abort_due_to_melee_threat():
		print("GOAP ACTION: MoveTo %s - ABORTING due to melee threat!" % target_var)
		agent.velocity = Vector2.ZERO
		return FAILURE
	var target_value = blackboard.get_var(target_var)
	if target_value == null or not is_instance_valid(target_value):
		print("GOAP ACTION: MoveTo %s - target invalid!" % target_var)
		return FAILURE
	var target_node: Node2D = target_value

	# Special handling for cover objects - use get_cover_position to position behind cover
	var target_pos: Vector2
	if target_var == &"cover_object" and agent.has_method("get_cover_position"):
		target_pos = agent.get_cover_position()
	else:
		target_pos = target_node.global_position
	var distance: float = agent.global_position.distance_to(target_pos)

	if distance < tolerance:
		agent.velocity = Vector2.ZERO
		# If moving to cover, enter cover when we arrive
		if target_var == &"cover_object" and agent.has_method("enter_cover"):
			agent.enter_cover()
		print("GOAP ACTION: MoveTo %s - reached destination at %s (target was %s)" % [target_var, agent.global_position, target_pos])
		return SUCCESS

	var direction: Vector2 = agent.global_position.direction_to(target_pos)
	agent.velocity = direction * speed

	# Update facing
	var scale_magnitude := absf(agent.root.scale.x)
	if direction.x > 0.1 and agent.root.scale.x < 0:
		agent.root.scale.x = scale_magnitude
	elif direction.x < -0.1 and agent.root.scale.x > 0:
		agent.root.scale.x = -scale_magnitude

	agent.animation_player.play(&"walk")
	return RUNNING


## Checks if a ranged agent should abort due to approaching melee threat
## Returns true if:
## 1. Agent is ranged (has ranged weapon)
## 2. Target (enemy) has melee weapon
## 3. Enemy is within danger distance AND approaching
func _should_abort_due_to_melee_threat() -> bool:
	# Only applies to ranged agents
	if not agent.has_node("CombatComponent"):
		return false
	var combat = agent.get_node("CombatComponent")
	if not combat.has_method("is_ranged") or not combat.is_ranged():
		return false

	# Check if enemy exists and has melee weapon
	var target: Node2D = blackboard.get_var(&"target", null)
	if not is_instance_valid(target):
		return false

	# Check if target has melee weapon
	var target_has_melee := false
	if target.has_node("CombatComponent"):
		var target_combat = target.get_node("CombatComponent")
		if target_combat.has_method("is_melee"):
			target_has_melee = target_combat.is_melee()
		elif target_combat.has_method("is_ranged"):
			target_has_melee = not target_combat.is_ranged()

	if not target_has_melee:
		return false

	# Check distance - use TOO_CLOSE_THRESHOLD as the danger distance
	var distance: float = agent.global_position.distance_to(target.global_position)
	var danger_dist := GOAPConfigClass.TOO_CLOSE_THRESHOLD

	if distance > danger_dist:
		return false  # Not close enough to worry

	# Check if enemy is approaching (getting closer)
	# Use velocity if available
	if "velocity" in target:
		var target_velocity: Vector2 = target.velocity
		if target_velocity.length() > 10.0:
			# Check if moving toward us
			var to_us: Vector2 = (agent.global_position - target.global_position).normalized()
			var approach_factor := target_velocity.normalized().dot(to_us)
			if approach_factor > 0.3:  # Moving toward us
				return true

	# Even if not moving, if very close, abort
	if distance < danger_dist * 0.6:
		return true

	return false
