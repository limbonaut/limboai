extends CharacterBody2D

## Base agent script.

@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var bt_player: BTPlayer = $BTPlayer
@onready var health: Health = $Health


func _ready() -> void:
	health.damaged.connect(_damaged)
	health.death.connect(_die)


func _damaged(_amount: float) -> void:
	animation_player.play(&"hit")


func _die() -> void:
	animation_player.play(&"death")
	bt_player.active = false
