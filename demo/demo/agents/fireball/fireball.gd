#*
#* fireball.gd
#* =============================================================================
#* Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
extends Node2D
## Fireball

const SPEED := 800.0
const DEAD_SPEED := 400.0

@export var dir: float = 1.0

var _is_dead: bool = false

@onready var fireball_sprite: Sprite2D = $Root/Fireball
@onready var death: GPUParticles2D = $FX/Death
@onready var collision_shape_2d: CollisionShape2D = $Hitbox/CollisionShape2D
@onready var root: Node2D = $Root
@onready var trail: GPUParticles2D = $FX/Trail


func _ready() -> void:
	var tween := create_tween().set_loops()
	tween.tween_property(fireball_sprite, ^"rotation", PI * signf(dir), 1.0).as_relative()

	var tween2 := create_tween().set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_IN)
	tween2.tween_property(fireball_sprite, "position:y", -10.0, 0.5).as_relative().set_ease(Tween.EASE_OUT)
	tween2.tween_property(fireball_sprite, "position:y", 0.0, 1.0)
	tween2.tween_callback(_die)


func _physics_process(delta: float) -> void:
	var speed: float = SPEED if not _is_dead else DEAD_SPEED
	position += Vector2.RIGHT * speed * dir * delta


func _die() -> void:
	if _is_dead:
		return
	_is_dead = true
	trail.emitting = false
	root.hide()
	collision_shape_2d.set_deferred(&"disabled", true)
	death.emitting = true
	await death.finished
	queue_free()


func _on_hitbox_area_entered(_area: Area2D) -> void:
	_die()
