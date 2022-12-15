@tool
extends BTAction

@export var animation_name: String
@export var player_path: NodePath

var _player: AnimationPlayer
var _finished: bool


func _generate_name() -> String:
	return "PlayAnimation \"%s\"" % animation_name 


func _setup() -> void:
	_player = agent.get_node(player_path)


func _enter() -> void:
	if _player.has_animation(animation_name):
		_finished = false
		_player.play(animation_name)
		_player.animation_finished.connect(_on_animation_finished, CONNECT_ONE_SHOT)
	else:
		_finished = true


func _tick(_delta: float) -> int:
	if _finished:
		return SUCCESS
	return RUNNING


func _on_animation_finished(_anim):
	_finished = true
