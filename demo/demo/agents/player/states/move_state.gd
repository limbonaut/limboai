extends LimboState

const VERTICAL_FACTOR := 0.8

## Move state.

@export var animation_player: AnimationPlayer
@export var animation: StringName
@export var speed: float = 500.0

func _enter() -> void:
	animation_player.play(animation, 0.1)


func _update(_delta: float) -> void:
	var horizontal_move: float = Input.get_axis(&"move_left", &"move_right")
	var vertical_move: float = Input.get_axis(&"move_up", &"move_down")

	agent.velocity = lerp(agent.velocity, Vector2(horizontal_move, vertical_move * VERTICAL_FACTOR) * speed, 0.2)
	agent.move_and_slide()

	if horizontal_move == 0.0 and vertical_move == 0.0:
		get_root().dispatch(EVENT_FINISHED)
