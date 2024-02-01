#*
#* get_first_in_group.gd
#* =============================================================================
#* Copyright 2021-2024 Serhii Snitsaruk
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
@tool
extends BTAction

## Get first node in group and save it to the blackboard.

@export var group: StringName
@export var output_var: String = "target"


func _generate_name() -> String:
	return "GetFirstNodeInGroup \"%s\"  ➜%s" % [
		group,
		LimboUtility.decorate_var(output_var)
		]

func _tick(_delta: float) -> Status:
	var node = agent.get_tree().get_first_node_in_group(group)
	blackboard.set_var(output_var, node)
	return SUCCESS
