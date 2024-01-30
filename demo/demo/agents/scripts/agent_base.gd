extends CharacterBody2D

## Base agent script.

@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var health: Health = $Health
@onready var root: Node2D = $Root

var _frames_since_facing_update: int = 0

func _ready() -> void:
	health.damaged.connect(_damaged)
	health.death.connect(_die)


func _physics_process(_delta: float) -> void:
	_update_facing()


func _update_facing() -> void:
	_frames_since_facing_update += 1
	if _frames_since_facing_update > 3:
		face_dir(velocity.x)


func face_dir(dir: float) -> void:
	if dir > 0.0 and root.scale.x < 0.0:
		root.scale.x = 1.0;
		_frames_since_facing_update = 0
	if dir < 0.0 and root.scale.x > 0.0:
		root.scale.x = -1.0;
		_frames_since_facing_update = 0


func _damaged(_amount: float) -> void:
	animation_player.play(&"hurt")
	var btplayer := get_node_or_null(^"BTPlayer") as BTPlayer
	if btplayer:
		btplayer.set_active(false)
	var hsm := get_node_or_null(^"LimboHSM")
	if hsm:
		hsm.set_active(false)
	await animation_player.animation_finished
	if btplayer:
		btplayer.restart()
	if hsm:
		hsm.set_active(true)



func _die() -> void:
	animation_player.play(&"death")

	for child in get_children():
		if child is BTPlayer or child is LimboHSM:
			child.set_active(false)

	await get_tree().create_timer(10.0).timeout
	queue_free()
