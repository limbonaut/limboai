@tool
extends BTAction
## Enters cover at the current position.

func _generate_name() -> String:
	return "GOAPTakeCover"


func _tick(_delta: float) -> Status:
	print("GOAP ACTION: TakeCover executed!")
	if agent.has_method("enter_cover"):
		agent.enter_cover()
		return SUCCESS
	return FAILURE
