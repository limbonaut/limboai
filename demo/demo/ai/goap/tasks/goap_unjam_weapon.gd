@tool
extends BTAction
## Unjams the weapon after a short delay.

## Time to unjam the weapon in seconds
@export var unjam_time := 0.8

var _elapsed := 0.0


func _generate_name() -> String:
	return "GOAPUnjamWeapon  time: %.1fs" % unjam_time


func _enter() -> void:
	_elapsed = 0.0


func _tick(delta: float) -> Status:
	_elapsed += delta

	if _elapsed < unjam_time:
		return RUNNING

	# Unjam the weapon
	if agent.has_method("unjam_weapon"):
		agent.unjam_weapon()

	return SUCCESS
