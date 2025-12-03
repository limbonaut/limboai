@tool
extends BTAction
## Leaves cover position.

func _generate_name() -> String:
	return "GOAPLeaveCover"


func _tick(_delta: float) -> Status:
	if agent.has_method("leave_cover"):
		agent.leave_cover()
		return SUCCESS
	return FAILURE
