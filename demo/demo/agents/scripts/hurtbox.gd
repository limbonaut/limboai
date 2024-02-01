#*
#* hurtbox.gd
#* =============================================================================
#* Copyright 2021-2024 Serhii Snitsaruk
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
class_name Hurtbox
extends Area2D
## Area that registers damage.


@export var health: Health


func take_damage(amount: float, knockback: Vector2) -> void:
	health.take_damage(amount, knockback)
