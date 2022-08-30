class_name BTPrintLine
extends BTTask


export var line: String


func _init(p_line: String = "") -> void:
	line = p_line


func _tick(_delta: float) -> int:
	print(line)
	return SUCCESS
