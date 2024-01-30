class_name Hitbox
extends Area2D

## Area that deals damage.

@export var damage: float = 1.0

func _ready() -> void:
	area_entered.connect(_on_area_entered)


func _on_area_entered(area: Area2D) -> void:
	var hurtbox := area as Hurtbox
	hurtbox.take_damage(damage, self)
