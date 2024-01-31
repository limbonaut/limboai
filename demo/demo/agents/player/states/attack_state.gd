extends LimboState

## Attack state: Perform 3-part combo attack for as long as player hits attack button.

@export var animation_player: AnimationPlayer
@export var animations: Array[StringName]

var attack_pressed: int


func _unhandled_input(event: InputEvent) -> void:
	if event.is_echo():
		return
	if event.is_action_pressed("attack"):
		attack_pressed += 1


func _enter() -> void:
	attack_pressed = 0
	for idx in animations.size():
		animation_player.play(animations[idx])
		await animation_player.animation_finished
		if attack_pressed <= 0 or not is_active():
			# Interrupt combo if player didn't press attack button again,
			# or state is no longer active.
			break
		attack_pressed -= 1
	if is_active():
		get_root().dispatch(EVENT_FINISHED)
