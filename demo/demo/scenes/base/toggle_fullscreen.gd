extends Button


func _ready() -> void:
	pressed.connect(_toggle_fullscreen)


func _toggle_fullscreen() -> void:
	if get_window().mode != Window.MODE_FULLSCREEN:
		get_window().mode = Window.MODE_FULLSCREEN
	else:
		get_window().mode = Window.MODE_WINDOWED
