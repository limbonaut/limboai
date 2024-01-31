class_name Hitbox
extends Area2D
## Area that deals damage.


@export var damage: float = 1.0


func _init() -> void:
	collision_layer = 0
	collision_mask = 4


func _ready() -> void:
	area_entered.connect(_area_entered)


func _area_entered(area: Area2D) -> void:
	var hurtbox := area as Hurtbox
	hurtbox.take_damage(damage, self)
