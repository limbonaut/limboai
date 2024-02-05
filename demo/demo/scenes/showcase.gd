extends Node2D

@onready var behavior_tree_view: BehaviorTreeView = %BehaviorTreeView
@onready var camera: Camera2D = $Camera2D
@onready var agent_selection: MenuButton = %AgentSelection
@onready var previous: Button = %Previous
@onready var next: Button = %Next

var bt_player: BTPlayer
var selected_tree_index: int = -1
var agent_files: Array[String]

func _ready() -> void:
	_populate_agent_files()
	_on_agent_selection_id_pressed(0)

	agent_selection.get_popup().id_pressed.connect(_on_agent_selection_id_pressed)
	previous.pressed.connect(func(): _on_agent_selection_id_pressed(selected_tree_index - 1))
	next.pressed.connect(func(): _on_agent_selection_id_pressed(selected_tree_index + 1))


func _physics_process(_delta: float) -> void:
	var inst: BTTask = bt_player.get_tree_instance()
	var bt_data: BehaviorTreeData = BehaviorTreeData.create_from_tree_instance(inst)
	behavior_tree_view.update_tree(bt_data)


func _attach_camera(agent: CharacterBody2D) -> void:
	await get_tree().process_frame
	camera.get_parent().remove_child(camera)
	agent.add_child(camera)
	camera.position = Vector2(400.0, 0.0)


func _populate_agent_files() -> void:
	var popup: PopupMenu = agent_selection.get_popup()
	popup.clear()
	popup.reset_size()
	agent_files.clear()

	var dir := DirAccess.open("res://demo/agents/")
	if dir:
		dir.list_dir_begin()
		var file_name: String = dir.get_next()
		while file_name != "":
			if dir.current_is_dir() or file_name.begins_with("agent_base"):
				file_name = dir.get_next()
				continue
			agent_files.append(file_name.get_file())
			file_name = dir.get_next()
	dir.list_dir_end()

	agent_files.sort()
	for i in agent_files.size():
		popup.add_item(agent_files[i], i)


func _load_agent(file_name: String) -> void:
	var agent_res := load(file_name) as PackedScene
	assert(agent_res != null)

	for child in get_children():
		if child is CharacterBody2D and child.name != "Dummy":
			child.die()

	var agent: CharacterBody2D = agent_res.instantiate()
	add_child(agent)
	bt_player = agent.find_child("BTPlayer")
	_attach_camera(agent)


func _on_agent_selection_id_pressed(id: int) -> void:
	assert(id >= 0 and id < agent_files.size())
	selected_tree_index = id
	_load_agent("res://demo/agents/".path_join(agent_files[id]))
	agent_selection.text = bt_player.behavior_tree.resource_path.get_file()
	previous.disabled = id == 0
	next.disabled = id == (agent_files.size()-1)


func _on_switch_to_game_pressed() -> void:
	get_tree().change_scene_to_file("res://demo/scenes/game.tscn")
