extends "res://demo/agents/scripts/agent_base.gd"

## Player.

@onready var limbo_hsm: LimboHSM = $LimboHSM
@onready var idle_state: LimboState = $LimboHSM/IdleState
@onready var move_state: LimboState = $LimboHSM/MoveState


func _ready() -> void:
	_init_state_machine()


func _init_state_machine() -> void:
	limbo_hsm.add_transition(idle_state, move_state, idle_state.EVENT_FINISHED)
	limbo_hsm.add_transition(move_state, idle_state, move_state.EVENT_FINISHED)
	limbo_hsm.initialize(self)
	limbo_hsm.set_active(true)
