class_name Hurtbox
extends Area2D
## Area that registers damage.


@export var health: Health


func _init() -> void:
	collision_layer = 4
	collision_mask = 0


func take_damage(amount: float, source: Area2D) -> void:
	if source.owner == owner:
		# Don't damage yourself.
		return
	health.take_damage(amount)
