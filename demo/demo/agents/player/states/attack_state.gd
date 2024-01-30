extends LimboState

## Idle state.

@export var animation_player: AnimationPlayer
@export var animation: StringName


func _enter() -> void:
	animation_player.play(animation)
	await animation_player.animation_finished
	if is_active():
		get_root().dispatch(EVENT_FINISHED)
