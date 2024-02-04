#*
#* attack_state.gd
#* =============================================================================
#* Copyright 2021-2024 Serhii Snitsaruk
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
extends LimboState

## Attack state: Perform 3-part combo attack for as long as player hits attack button.

@export var animation_player: AnimationPlayer
@export var animations: Array[StringName]
@export var hitbox: Hitbox

var attack_pressed: int


func _unhandled_input(event: InputEvent) -> void:
	if event.is_echo():
		return
	if event.is_action_pressed("attack"):
		attack_pressed += 1


func _enter() -> void:
	attack_pressed = 0
	hitbox.damage = 1
	for idx in animations.size():
		hitbox.damage = 2 if idx == 2 else 1  # deal 2 damage on third attack
		animation_player.play(animations[idx])
		await animation_player.animation_finished
		if attack_pressed <= 0 or not is_active():
			# Interrupt combo if player didn't press attack button again,
			# or state is no longer active.
			break
		attack_pressed -= 1
	if is_active():
		get_root().dispatch(EVENT_FINISHED)


func _exit() -> void:
	hitbox.damage = 1
