@tool
extends BTAction
## Leaves cover position - stands up from crouching.

func _generate_name() -> String:
	return "GOAPLeaveCover"


func _enter() -> void:
	print("GOAP ACTION: LeaveCover started - standing up from cover")


func _tick(_delta: float) -> Status:
	if agent.has_method("leave_cover"):
		agent.leave_cover()
		print("GOAP ACTION: LeaveCover completed - now mobile")
		return SUCCESS
	return FAILURE
