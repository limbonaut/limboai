#*
#* hitbox.gd
#* =============================================================================
#* Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
class_name Hitbox
extends Area2D
## Area that deals damage.

## Damage value to apply.
@export var damage: float = 1.0

## Push back the victim.
@export var knockback_enabled: bool = false

## Desired pushback speed.
@export var knockback_strength: float = 500.0


func _ready() -> void:
	area_entered.connect(_area_entered)


func _area_entered(hurtbox: Hurtbox) -> void:
	if hurtbox.owner == owner:
		return
	# Check if this hitbox belongs to a projectile with a shooter
	var projectile_owner = owner
	if projectile_owner:
		var shooter = projectile_owner.get(&"shooter") if projectile_owner.has_method(&"get") else null
		if shooter == null and "shooter" in projectile_owner:
			shooter = projectile_owner.shooter
		if shooter and is_instance_valid(shooter):
			if hurtbox.owner == shooter:
				print("Hitbox: Ignoring damage to shooter %s" % shooter.name)
				return  # Don't damage the shooter
	hurtbox.take_damage(damage, get_knockback(), self)


func get_knockback() -> Vector2:
	var knockback: Vector2
	if knockback_enabled:
		knockback = Vector2.RIGHT.rotated(global_rotation) * knockback_strength
	return knockback
