extends LimboState
## Dodge state.


@export var animation_player: AnimationPlayer
@export var animation: StringName
@export var duration: float = 0.4
@export var dodge_speed: float = 1000.0
@export var hurtbox_collision: CollisionShape2D

var move_dir: Vector2
var elapsed_time: float


func _enter() -> void:
	elapsed_time = 0.0
	hurtbox_collision.disabled = true

	var horizontal_move: float = Input.get_axis(&"move_left", &"move_right")
	if is_zero_approx(horizontal_move):
		move_dir = Vector2.RIGHT * agent.get_facing()
	else:
		move_dir = Vector2.RIGHT * signf(horizontal_move)
	agent.face_dir(move_dir.x)

	animation_player.play(animation, 0.1)


func _exit() -> void:
	hurtbox_collision.disabled = false


func _update(p_delta: float) -> void:
	elapsed_time += p_delta
	var desired_velocity: Vector2 = move_dir * dodge_speed
	agent.velocity = lerp(agent.velocity, desired_velocity, 0.2)
	agent.move_and_slide()
	if elapsed_time > duration:
		get_root().dispatch(EVENT_FINISHED)
