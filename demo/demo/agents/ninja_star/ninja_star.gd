#*
#* ninja_star.gd
#* =============================================================================
#* Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
extends Node2D

const SPEED := 800.0
const DEAD_SPEED := 400.0
const COVER_COLLISION_LAYER := 16  # Layer for cover objects

## Direction vector for projectile movement (normalized)
@export var direction: Vector2 = Vector2.RIGHT

## Legacy float direction for backwards compatibility
@export var dir: float = 1.0

## The node that fired this projectile (to prevent self-damage)
var shooter: Node = null

var _is_dead: bool = false
var _spin_tween: Tween
var _arc_tween: Tween
var _raycast: RayCast2D

@onready var ninja_star: Sprite2D = $Root/NinjaStar
@onready var death: GPUParticles2D = $Death
@onready var collision_shape_2d: CollisionShape2D = $Hitbox/CollisionShape2D
@onready var root: Node2D = $Root


func _ready() -> void:
	add_to_group("projectiles")

	# Disable hitbox initially to prevent self-damage, enable after short delay
	collision_shape_2d.disabled = true
	get_tree().create_timer(0.05).timeout.connect(func():
		if is_instance_valid(collision_shape_2d):
			collision_shape_2d.disabled = false
	)

	# Create raycast for cover detection
	_raycast = RayCast2D.new()
	_raycast.collision_mask = COVER_COLLISION_LAYER
	_raycast.target_position = direction * 50  # Look ahead
	add_child(_raycast)

	# If direction wasn't set but dir was, use legacy horizontal movement
	if direction == Vector2.RIGHT and dir != 1.0:
		direction = Vector2.RIGHT * dir

	_spin_tween = create_tween().set_loops()
	_spin_tween.tween_property(ninja_star, ^"rotation", TAU * signf(direction.x), 1.0).as_relative()

	# Rotate the sprite to face the movement direction
	root.rotation = direction.angle()

	_arc_tween = create_tween().set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_IN)
	_arc_tween.tween_property(ninja_star, "position:y", -10.0, 0.5).as_relative().set_ease(Tween.EASE_OUT)
	_arc_tween.tween_property(ninja_star, "position:y", 0.0, 1.0)
	_arc_tween.tween_callback(_die)


func _physics_process(delta: float) -> void:
	var speed: float = SPEED if not _is_dead else DEAD_SPEED
	position += direction * speed * delta

	# Check if we hit cover
	if _raycast and _raycast.is_colliding():
		print("Projectile blocked by cover!")
		_die()


func _die(skip_particles: bool = false) -> void:
	if _is_dead:
		return
	_is_dead = true

	# Kill tweens to prevent resource leaks
	if _spin_tween and _spin_tween.is_valid():
		_spin_tween.kill()
	if _arc_tween and _arc_tween.is_valid():
		_arc_tween.kill()

	root.hide()
	collision_shape_2d.set_deferred(&"disabled", true)
	if skip_particles:
		queue_free()
		return

	# Use timer instead of await to avoid coroutine accumulation
	death.emitting = true
	get_tree().create_timer(death.lifetime).timeout.connect(queue_free)
	#await death.finished
	#queue_free()


func _on_screen_exited() -> void:
	_die(true)  # Skip particles for off-screen cleanup


func _on_hitbox_area_entered(_area: Area2D) -> void:
	_die()  # Show particles on hit
