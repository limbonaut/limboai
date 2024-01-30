class_name Hurtbox
extends Area2D

## Area that registers damage.

@export var health: Health

func take_damage(amount: float, source: Area2D) -> void:
	if source.owner == owner:
		# Don't damage yourself.
		return
	health.take_damage(amount)
