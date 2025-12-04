## Simple pickup item for GOAP demo
## Supports respawning after being collected
extends Node2D

@export var pickup_type: String = "weapon"  # "weapon", "ammo", or "health"
@export var bob_speed: float = 2.0
@export var bob_height: float = 8.0
@export var respawn_time: float = 5.0  # Time in seconds before respawning (0 = no respawn)
@export var random_respawn: bool = false  # If true, respawn at random location
@export var random_initial_spawn: bool = false  # If true, start at random location
@export var respawn_area_min: Vector2 = Vector2(200, 200)  # Min bounds for random respawn
@export var respawn_area_max: Vector2 = Vector2(900, 600)  # Max bounds for random respawn
@export var avoid_node_path: NodePath  # Node to avoid when respawning (e.g., cover)
@export var avoid_distance: float = 200.0  # Minimum distance from avoided node

var _initial_y: float
var _time: float = 0.0
var _is_collected: bool = false

@onready var sprite: Sprite2D = $Sprite2D
@onready var label: Label = $Label


func _ready() -> void:
	# Randomize initial position if enabled
	if random_initial_spawn:
		var new_pos := _get_valid_respawn_position()
		position = new_pos
		_initial_y = new_pos.y
		print("PICKUP: %s spawned at random location (%.0f, %.0f)" % [pickup_type, new_pos.x, new_pos.y])
	else:
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

	# Move to random location if enabled
	if random_respawn:
		var new_pos := _get_valid_respawn_position()
		position = new_pos
		_initial_y = new_pos.y
		print("PICKUP: %s respawned at random location (%.0f, %.0f)!" % [pickup_type, new_pos.x, new_pos.y])
	else:
		print("PICKUP: %s respawned!" % pickup_type)


func _get_valid_respawn_position() -> Vector2:
	var avoid_node: Node2D = null
	if avoid_node_path and not avoid_node_path.is_empty():
		avoid_node = get_node_or_null(avoid_node_path) as Node2D

	# Try up to 20 times to find a valid position
	for _attempt in range(20):
		var new_x := randf_range(respawn_area_min.x, respawn_area_max.x)
		var new_y := randf_range(respawn_area_min.y, respawn_area_max.y)
		var new_pos := Vector2(new_x, new_y)

		# Check if position is far enough from avoided node
		if avoid_node == null or new_pos.distance_to(avoid_node.global_position) >= avoid_distance:
			return new_pos

	# Fallback: just return a random position if we couldn't find a valid one
	return Vector2(
		randf_range(respawn_area_min.x, respawn_area_max.x),
		randf_range(respawn_area_min.y, respawn_area_max.y)
	)


## Returns true if this pickup can be collected
func is_available() -> bool:
	return not _is_collected
