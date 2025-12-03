@tool
extends BTAction
## Waits in cover until the threat passes.
## Returns RUNNING while under threat, SUCCESS when threat ends.


func _generate_name() -> String:
	return "GOAPWaitInCover"


func _enter() -> void:
	print("GOAP ACTION: WaitInCover started - staying in cover until threat passes")


func _tick(_delta: float) -> Status:
	var under_threat: bool = blackboard.get_var(&"under_threat", false)
	var in_cover: bool = blackboard.get_var(&"in_cover", false)

	if not in_cover:
		print("GOAP ACTION: WaitInCover - not in cover anymore!")
		return FAILURE

	if under_threat:
		# Still waiting for threat to pass
		agent.animation_player.play(&"idle")
		agent.velocity = Vector2.ZERO
		return RUNNING

	print("GOAP ACTION: WaitInCover - threat has passed!")
	return SUCCESS
