extends "res://demo/agents/scripts/agent_base.gd"

## Player.

@onready var hsm: LimboHSM = $LimboHSM
@onready var idle_state: LimboState = $LimboHSM/IdleState
@onready var move_state: LimboState = $LimboHSM/MoveState
@onready var attack_state: LimboState = $LimboHSM/AttackState


func _ready() -> void:
	super._ready()
	_init_state_machine()


func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("attack") and event.is_echo() == false:
		hsm.dispatch("attack!")


func _init_state_machine() -> void:
	hsm.add_transition(idle_state, move_state, idle_state.EVENT_FINISHED)
	hsm.add_transition(move_state, idle_state, move_state.EVENT_FINISHED)
	hsm.add_transition(idle_state, attack_state, "attack!")
	hsm.add_transition(move_state, attack_state, "attack!")
	hsm.add_transition(attack_state, move_state, attack_state.EVENT_FINISHED)
	hsm.initialize(self)
	hsm.set_active(true)
