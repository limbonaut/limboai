class_name Health
extends Node
## Tracks health and emits signal when damaged or dead.


signal death
signal damaged(amount: float)

@export var max_health: float = 10.0

var _current: float


func _ready() -> void:
	_current = max_health


func take_damage(amount: float) -> void:
	if _current <= 0:
		return

	_current -= amount
	_current = max(_current, 0.0)

	if _current <= 0.0:
		death.emit()
	else:
		damaged.emit(amount)


func get_current() -> float:
	return _current
