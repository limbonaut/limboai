## Simple pickup item for GOAP demo
## Supports respawning after being collected
extends Node2D

@export var pickup_type: String = "weapon"  # "weapon", "ammo", or "health"
@export var bob_speed: float = 2.0
@export var bob_height: float = 8.0
@export var respawn_time: float = 5.0  # Time in seconds before respawning (0 = no respawn)

var _initial_y: float
var _time: float = 0.0
var _is_collected: bool = false

@onready var sprite: Sprite2D = $Sprite2D
@onready var label: Label = $Label


func _ready() -> void:
	_initial_y = position.y
	if label:
		label.text = pickup_type.capitalize()


func _process(delta: float) -> void:
	if _is_collected:
		return
	_time += delta
	position.y = _initial_y + sin(_time * bob_speed) * bob_height


## Called when this pickup is collected
func collect() -> void:
	if _is_collected:
		return
	_is_collected = true
	visible = false
	set_process(false)

	if respawn_time > 0:
		# Start respawn timer
		var timer := get_tree().create_timer(respawn_time)
		timer.timeout.connect(_respawn)
		print("PICKUP: %s collected, will respawn in %.1fs" % [pickup_type, respawn_time])
	else:
		print("PICKUP: %s collected (no respawn)" % pickup_type)


func _respawn() -> void:
	_is_collected = false
	visible = true
	set_process(true)
	print("PICKUP: %s respawned!" % pickup_type)


## Returns true if this pickup can be collected
func is_available() -> bool:
	return not _is_collected
