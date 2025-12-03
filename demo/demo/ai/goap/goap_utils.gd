## GOAP Utility Functions
## Shared helper functions for GOAP demo
class_name GOAPUtils
extends RefCounted


## Recursively finds the BTRunGOAPPlan task in a behavior tree
static func find_goap_task(task) -> Variant:
	if task == null:
		return null

	# Check if this task is BTRunGOAPPlan
	if task.get_class() == "BTRunGOAPPlan":
		return task

	# Check children
	var child_count = task.get_child_count()
	for i in range(child_count):
		var child = task.get_child(i)
		var result = find_goap_task(child)
		if result:
			return result

	return null


## Finds GOAP task from a BTPlayer node
static func find_goap_task_from_player(bt_player: BTPlayer) -> Variant:
	if not bt_player:
		return null

	var bt_instance = bt_player.get_bt_instance()
	if not bt_instance:
		return null

	var root_task = bt_instance.get_root_task()
	if not root_task:
		return null

	return find_goap_task(root_task)


## Formats a boolean for display (YES/no style)
static func bool_str(value: bool) -> String:
	return "YES" if value else "no"
