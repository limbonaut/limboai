@tool
extends Marker2D


func _ready() -> void:
	queue_redraw()


func _draw() -> void:
	draw_circle(Vector2.ZERO, 50.0, Color.CHARTREUSE)
