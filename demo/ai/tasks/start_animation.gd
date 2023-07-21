#*
#* start_animation.gd
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


func _generate_name() -> String:
	return "StartAnimation \"%s\"" % animation_name


func _setup() -> void:
	_player = agent.get_node(animation_player)


func _tick(p_delta: float) -> int:
	_player.play(animation_name)
	return SUCCESS
