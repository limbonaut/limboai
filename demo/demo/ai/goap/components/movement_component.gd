## Movement Component
## Handles agent movement and facing direction
class_name MovementComponent
extends Node

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

@export var move_speed: float = GOAPConfigClass.MOVE_SPEED
@export var velocity_lerp := 0.2

var agent: CharacterBody2D
var root: Node2D  # Visual root for flipping


func _ready() -> void:
	# Try to find agent and root automatically if not set
	if not agent:
		agent = get_parent() as CharacterBody2D
	if agent and not root:
		root = agent.get_node_or_null("Root") as Node2D


## Moves the agent with velocity smoothing
func move(p_velocity: Vector2) -> void:
	if not agent:
		return
	agent.velocity = lerp(agent.velocity, p_velocity, velocity_lerp)
	agent.move_and_slide()
	_clamp_to_arena()
	update_facing()


## Keeps the agent within arena bounds
func _clamp_to_arena() -> void:
	if not agent:
		return
	agent.global_position.x = clampf(agent.global_position.x, GOAPConfigClass.ARENA_MIN.x, GOAPConfigClass.ARENA_MAX.x)
	agent.global_position.y = clampf(agent.global_position.y, GOAPConfigClass.ARENA_MIN.y, GOAPConfigClass.ARENA_MAX.y)


## Moves toward a target position at move_speed
func move_toward(target_pos: Vector2) -> void:
	if not agent:
		return
	var direction := agent.global_position.direction_to(target_pos)
	move(direction * move_speed)


## Stops movement
func stop() -> void:
	if agent:
		agent.velocity = Vector2.ZERO


## Updates facing direction based on velocity
func update_facing() -> void:
	if not agent or not root:
		return
	var scale_magnitude := absf(root.scale.x)
	if agent.velocity.x > 10 and root.scale.x < 0:
		root.scale.x = scale_magnitude
	elif agent.velocity.x < -10 and root.scale.x > 0:
		root.scale.x = -scale_magnitude


## Returns current facing direction (1 = right, -1 = left)
func get_facing() -> float:
	if root:
		return signf(root.scale.x)
	return 1.0


## Face toward a target position
func face_toward(target_pos: Vector2) -> void:
	if not agent or not root:
		return
	var dir := target_pos.x - agent.global_position.x
	var scale_magnitude := absf(root.scale.x)
	if dir > 0:
		root.scale.x = scale_magnitude
	elif dir < 0:
		root.scale.x = -scale_magnitude
