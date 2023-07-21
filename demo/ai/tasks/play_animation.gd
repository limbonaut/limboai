#*
#* play_animation.gd
#* =============================================================================
#* Copyright 2021-2023 Serhii Snitsaruk
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*

@tool
extends BTAction

@export var animation_name: String
@export var animation_player: NodePath

var _player: AnimationPlayer
var _finished: bool


func _generate_name() -> String:
	return "PlayAnimation \"%s\"" % animation_name


func _setup() -> void:
	_player = agent.get_node(animation_player)


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
