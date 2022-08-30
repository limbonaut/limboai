extends KinematicBody2D


onready var bt_player: BTPlayer = $BTPlayer


func _ready() -> void:
	_configure_ai()


func _configure_ai() -> void:
	var tree := BehaviorTree.new()
	var seq := BTSequence.new()
	var print_task := BTPrintLine.new("Hello world!")
	seq.add_child(print_task)
	tree.root_task = seq
	bt_player.behavior_tree = tree
	print("Assigning tree second time")
	bt_player.behavior_tree = tree
