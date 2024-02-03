extends Node2D

@onready var behavior_tree_view: BehaviorTreeView = %BehaviorTreeView
@onready var camera: Camera2D = $Camera2D
@onready var resource_name: Label = $CanvasLayer/ResourceName

var bt_player: BTPlayer

func _ready() -> void:
	var agent: CharacterBody2D
	for child in get_children():
		if child is CharacterBody2D:
			bt_player = child.find_child("BTPlayer")
			if bt_player != null:
				agent = child
				resource_name.text = bt_player.behavior_tree.resource_path.get_file()
				break
	_attach_camera(agent)


func _physics_process(_delta: float) -> void:
	var inst: BTTask = bt_player.get_tree_instance()
	var bt_data: BehaviorTreeData = BehaviorTreeData.create_from_tree_instance(inst)
	behavior_tree_view.update_tree(bt_data)


func _attach_camera(agent: CharacterBody2D) -> void:
	await get_tree().process_frame
	camera.get_parent().remove_child(camera)
	agent.add_child(camera)
	camera.position = Vector2(400.0, 0.0)
