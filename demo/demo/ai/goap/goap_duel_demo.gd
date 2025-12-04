## GOAP Duel Demo Manager
## Two GOAP agents fighting each other - no scripted enemy
## Demonstrates emergent tactical behavior from GOAP planning
extends Node2D

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const GOAPUtilsClass = preload("res://demo/ai/goap/goap_utils.gd")

@onready var agent_red: CharacterBody2D = $AgentRed
@onready var agent_blue: CharacterBody2D = $AgentBlue

# Pickups
@onready var weapon_pickup_red: Node2D = $WeaponPickupRed
@onready var weapon_pickup_blue: Node2D = $WeaponPickupBlue
@onready var ammo_pickup: Node2D = $AmmoPickup
@onready var health_pickup: Node2D = $HealthPickup

# UI elements
@onready var red_health_label: Label = %RedHealthLabel
@onready var blue_health_label: Label = %BlueHealthLabel
@onready var red_status_label: Label = %RedStatusLabel
@onready var blue_status_label: Label = %BlueStatusLabel
@onready var winner_label: Label = %WinnerLabel

# Cover objects - found at runtime
var cover_objects: Array[Node2D] = []

# Match state
var match_over := false
var winner_name := ""


func _ready() -> void:
	# Find all cover objects in the scene
	_find_cover_objects()

	# Set up agents to fight each other
	_setup_agent_vs_agent()

	# Connect signals
	agent_red.health_changed.connect(_on_red_health_changed)
	agent_blue.health_changed.connect(_on_blue_health_changed)
	agent_red.died.connect(_on_red_died)
	agent_blue.died.connect(_on_blue_died)

	# Hide winner label
	winner_label.visible = false

	print("GOAP Duel: Two agents ready to fight!")


func _find_cover_objects() -> void:
	# Find all CoverObject nodes in the scene
	for child in get_children():
		if child.name.begins_with("Cover"):
			cover_objects.append(child)

	print("GOAP Duel: Found %d cover objects" % cover_objects.size())


func _setup_agent_vs_agent() -> void:
	# Red targets Blue
	agent_red.set_target(agent_blue)
	agent_red.set_cover_objects_array(cover_objects)
	agent_red.agent_name = "Red"
	agent_red.weapon_pickup = weapon_pickup_red
	agent_red.ammo_pickup = ammo_pickup
	agent_red.health_pickup = health_pickup
	if agent_red.world_state:
		agent_red.world_state.weapon_pickup = weapon_pickup_red
		agent_red.world_state.ammo_pickup = ammo_pickup
		agent_red.world_state.health_pickup = health_pickup

	# Blue targets Red
	agent_blue.set_target(agent_red)
	agent_blue.set_cover_objects_array(cover_objects)
	agent_blue.agent_name = "Blue"
	agent_blue.weapon_pickup = weapon_pickup_blue
	agent_blue.ammo_pickup = ammo_pickup
	agent_blue.health_pickup = health_pickup
	if agent_blue.world_state:
		agent_blue.world_state.weapon_pickup = weapon_pickup_blue
		agent_blue.world_state.ammo_pickup = ammo_pickup
		agent_blue.world_state.health_pickup = health_pickup


func _process(_delta: float) -> void:
	if match_over:
		return

	_update_status_displays()


func _update_status_displays() -> void:
	# Update Red status
	var red_goal := _get_agent_goal_name(agent_red)
	var red_action := _get_agent_current_action(agent_red)
	red_status_label.text = "Goal: %s\nAction: %s" % [red_goal, red_action]

	# Update Blue status
	var blue_goal := _get_agent_goal_name(agent_blue)
	var blue_action := _get_agent_current_action(agent_blue)
	blue_status_label.text = "Goal: %s\nAction: %s" % [blue_goal, blue_action]


func _get_agent_goal_name(agent: CharacterBody2D) -> String:
	var bt_player: BTPlayer = agent.get_node_or_null("BTPlayer")
	if not bt_player:
		return "?"

	var goap_task = GOAPUtilsClass.find_goap_task_from_player(bt_player)
	if goap_task and goap_task.has_method("get_goal"):
		var goal = goap_task.get_goal()
		if goal:
			return goal.goal_name
	return "Planning..."


func _get_agent_current_action(agent: CharacterBody2D) -> String:
	var bt_player: BTPlayer = agent.get_node_or_null("BTPlayer")
	if not bt_player:
		return "?"

	var goap_task = GOAPUtilsClass.find_goap_task_from_player(bt_player)
	if goap_task:
		var plan = goap_task.get_current_plan()
		var idx = goap_task.get_current_action_index()
		if plan.size() > 0 and idx < plan.size():
			return plan[idx].action_name
	return "None"


func _on_red_health_changed(current: int, max_hp: int) -> void:
	red_health_label.text = "HP: %d/%d" % [current, max_hp]
	_update_health_color(red_health_label, current, max_hp)


func _on_blue_health_changed(current: int, max_hp: int) -> void:
	blue_health_label.text = "HP: %d/%d" % [current, max_hp]
	_update_health_color(blue_health_label, current, max_hp)


func _update_health_color(label: Label, current: int, max_hp: int) -> void:
	var ratio := float(current) / float(max_hp)
	if ratio > 0.5:
		label.modulate = Color.WHITE
	elif ratio > 0.25:
		label.modulate = Color.YELLOW
	else:
		label.modulate = Color.RED


func _on_red_died() -> void:
	if not match_over:
		_end_match("Blue")


func _on_blue_died() -> void:
	if not match_over:
		_end_match("Red")


func _end_match(winner: String) -> void:
	match_over = true
	winner_name = winner
	winner_label.text = "%s WINS!" % winner.to_upper()
	winner_label.visible = true

	# Color the winner label
	if winner == "Red":
		winner_label.modulate = Color(1, 0.3, 0.3)
	else:
		winner_label.modulate = Color(0.3, 0.5, 1)

	print("GOAP Duel: %s wins!" % winner)


func _on_restart_pressed() -> void:
	print("GOAP Duel: Restarting...")
	get_tree().reload_current_scene()
