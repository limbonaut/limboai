#*
#* player.gd
#* =============================================================================
#* Copyright 2021-2024 Serhii Snitsaruk
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
extends "res://demo/agents/scripts/agent_base.gd"

## Player.

@export var dash_cooldown: float = 0.4

@onready var hsm: LimboHSM = $LimboHSM
@onready var idle_state: LimboState = $LimboHSM/IdleState
@onready var move_state: LimboState = $LimboHSM/MoveState
@onready var attack_state: LimboState = $LimboHSM/AttackState
@onready var dash_state: LimboState = $LimboHSM/DashState

var can_dash: bool = true


func _ready() -> void:
	super._ready()
	can_dash = true
	_init_state_machine()
	death.connect(func(): remove_from_group(&"player"))


func _unhandled_input(event: InputEvent) -> void:
	if event.is_echo():
		return
	if event.is_action_pressed("attack"):
		hsm.dispatch("attack!")
	if event.is_action_pressed("dash"):
		hsm.dispatch("dash!")


func _init_state_machine() -> void:
	hsm.add_transition(idle_state, move_state, idle_state.EVENT_FINISHED)
	hsm.add_transition(move_state, idle_state, move_state.EVENT_FINISHED)
	hsm.add_transition(idle_state, attack_state, "attack!")
	hsm.add_transition(move_state, attack_state, "attack!")
	hsm.add_transition(attack_state, move_state, attack_state.EVENT_FINISHED)
	hsm.add_transition(hsm.ANYSTATE, dash_state, "dash!")
	hsm.add_transition(dash_state, move_state, dash_state.EVENT_FINISHED)
	dash_state.set_guard(_can_dash)
	hsm.initialize(self)
	hsm.set_active(true)
	hsm.set_guard(_can_dash)


func set_victorious() -> void:
	idle_state.idle_animation = &"dance"


func _can_dash() -> bool:
	if can_dash:
		can_dash = false
		get_tree().create_timer(dash_cooldown).timeout.connect(func(): can_dash = true)
		return true
	return false
