## GOAP Duel Demo Manager
## Supports 1v1 or 2v2 team battles between GOAP agents
## Demonstrates emergent tactical behavior from GOAP planning
extends Node2D

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")
const GOAPUtilsClass = preload("res://demo/ai/goap/goap_utils.gd")

# Team Red agents
var red_team: Array[CharacterBody2D] = []
# Team Blue agents
var blue_team: Array[CharacterBody2D] = []

# Optional direct references for 1v1 mode
@onready var agent_red: CharacterBody2D = $AgentRed if has_node("AgentRed") else null
@onready var agent_blue: CharacterBody2D = $AgentBlue if has_node("AgentBlue") else null

# Additional agents for 2v2
@onready var agent_red2: CharacterBody2D = $AgentRed2 if has_node("AgentRed2") else null
@onready var agent_blue2: CharacterBody2D = $AgentBlue2 if has_node("AgentBlue2") else null

# Pickups
@onready var weapon_pickup_red: Node2D = $WeaponPickupRed if has_node("WeaponPickupRed") else null
@onready var weapon_pickup_blue: Node2D = $WeaponPickupBlue if has_node("WeaponPickupBlue") else null
@onready var ammo_pickup: Node2D = $AmmoPickup if has_node("AmmoPickup") else null
@onready var health_pickup: Node2D = $HealthPickup if has_node("HealthPickup") else null

# Additional pickups for 2v2
@onready var ammo_pickup2: Node2D = $AmmoPickup2 if has_node("AmmoPickup2") else null
@onready var health_pickup2: Node2D = $HealthPickup2 if has_node("HealthPickup2") else null

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
var is_team_mode := false


func _ready() -> void:
	# Find all cover objects in the scene
	_find_cover_objects()

	# Build teams
	_build_teams()

	# Set up agent targeting
	_setup_team_targeting()

	# Connect signals
	_connect_agent_signals()

	# Hide winner label
	winner_label.visible = false

	var mode := "2v2 Team Battle" if is_team_mode else "1v1 Duel"
	print("GOAP Duel: %s - %d Red vs %d Blue!" % [mode, red_team.size(), blue_team.size()])


func _find_cover_objects() -> void:
	for child in get_children():
		if child.name.begins_with("Cover"):
			cover_objects.append(child)
	print("GOAP Duel: Found %d cover objects" % cover_objects.size())


func _build_teams() -> void:
	# Add Red team members
	if agent_red:
		red_team.append(agent_red)
		agent_red.team = 0
	if agent_red2:
		red_team.append(agent_red2)
		agent_red2.team = 0

	# Add Blue team members
	if agent_blue:
		blue_team.append(agent_blue)
		agent_blue.team = 1
	if agent_blue2:
		blue_team.append(agent_blue2)
		agent_blue2.team = 1

	# Determine if this is team mode
	is_team_mode = red_team.size() > 1 or blue_team.size() > 1


func _setup_team_targeting() -> void:
	# Set up Red team
	for agent in red_team:
		agent.set_enemies(blue_team)
		agent.set_cover_objects_array(cover_objects)
		_setup_agent_pickups(agent, weapon_pickup_red)

	# Set up Blue team
	for agent in blue_team:
		agent.set_enemies(red_team)
		agent.set_cover_objects_array(cover_objects)
		_setup_agent_pickups(agent, weapon_pickup_blue)


func _setup_agent_pickups(agent: CharacterBody2D, weapon: Node2D) -> void:
	agent.weapon_pickup = weapon
	agent.ammo_pickup = ammo_pickup
	agent.health_pickup = health_pickup

	if agent.world_state:
		agent.world_state.weapon_pickup = weapon
		agent.world_state.ammo_pickup = ammo_pickup
		agent.world_state.health_pickup = health_pickup


func _connect_agent_signals() -> void:
	for agent in red_team:
		agent.health_changed.connect(_on_red_health_changed)
		agent.died.connect(_on_agent_died.bind(agent, "Red"))

	for agent in blue_team:
		agent.health_changed.connect(_on_blue_health_changed)
		agent.died.connect(_on_agent_died.bind(agent, "Blue"))


func _process(_delta: float) -> void:
	if match_over:
		return
	_update_status_displays()


func _update_status_displays() -> void:
	# Update Red status (show first alive agent or last agent)
	var red_agent := _get_display_agent(red_team)
	if red_agent:
		var red_goal := _get_agent_goal_name(red_agent)
		var red_action := _get_agent_current_action(red_agent)
		var alive_count := _count_alive(red_team)
		var team_info := " (%d alive)" % alive_count if is_team_mode else ""
		red_status_label.text = "Goal: %s\nAction: %s%s" % [red_goal, red_action, team_info]

	# Update Blue status
	var blue_agent := _get_display_agent(blue_team)
	if blue_agent:
		var blue_goal := _get_agent_goal_name(blue_agent)
		var blue_action := _get_agent_current_action(blue_agent)
		var alive_count := _count_alive(blue_team)
		var team_info := " (%d alive)" % alive_count if is_team_mode else ""
		blue_status_label.text = "Goal: %s\nAction: %s%s" % [blue_goal, blue_action, team_info]


func _get_display_agent(team: Array[CharacterBody2D]) -> CharacterBody2D:
	# Return first alive agent, or last agent if all dead
	for agent in team:
		if is_instance_valid(agent) and agent.health > 0:
			return agent
	return team[0] if team.size() > 0 else null


func _count_alive(team: Array[CharacterBody2D]) -> int:
	var count := 0
	for agent in team:
		if is_instance_valid(agent) and agent.health > 0:
			count += 1
	return count


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
	if is_team_mode:
		var total_hp := _get_team_total_health(red_team)
		var total_max := _get_team_max_health(red_team)
		red_health_label.text = "HP: %d/%d" % [total_hp, total_max]
		_update_health_color(red_health_label, total_hp, total_max)
	else:
		red_health_label.text = "HP: %d/%d" % [current, max_hp]
		_update_health_color(red_health_label, current, max_hp)


func _on_blue_health_changed(current: int, max_hp: int) -> void:
	if is_team_mode:
		var total_hp := _get_team_total_health(blue_team)
		var total_max := _get_team_max_health(blue_team)
		blue_health_label.text = "HP: %d/%d" % [total_hp, total_max]
		_update_health_color(blue_health_label, total_hp, total_max)
	else:
		blue_health_label.text = "HP: %d/%d" % [current, max_hp]
		_update_health_color(blue_health_label, current, max_hp)


func _get_team_total_health(team: Array[CharacterBody2D]) -> int:
	var total := 0
	for agent in team:
		if is_instance_valid(agent):
			total += max(0, agent.health)
	return total


func _get_team_max_health(team: Array[CharacterBody2D]) -> int:
	var total := 0
	for agent in team:
		if is_instance_valid(agent):
			total += agent.max_health
	return total


func _update_health_color(label: Label, current: int, max_hp: int) -> void:
	var ratio := float(current) / float(max_hp) if max_hp > 0 else 0.0
	if ratio > 0.5:
		label.modulate = Color.WHITE
	elif ratio > 0.25:
		label.modulate = Color.YELLOW
	else:
		label.modulate = Color.RED


func _on_agent_died(agent: CharacterBody2D, team_name: String) -> void:
	print("GOAP Duel: %s agent %s died!" % [team_name, agent.agent_name])

	# Check if entire team is eliminated
	if team_name == "Red":
		if _count_alive(red_team) == 0:
			_end_match("Blue")
	else:
		if _count_alive(blue_team) == 0:
			_end_match("Red")


func _end_match(winner: String) -> void:
	if match_over:
		return

	match_over = true
	winner_name = winner
	var suffix := " TEAM WINS!" if is_team_mode else " WINS!"
	winner_label.text = "%s%s" % [winner.to_upper(), suffix]
	winner_label.visible = true

	# Color the winner label
	if winner == "Red":
		winner_label.modulate = Color(1, 0.3, 0.3)
	else:
		winner_label.modulate = Color(0.3, 0.5, 1)

	print("GOAP Duel: %s%s" % [winner, suffix])


func _on_restart_pressed() -> void:
	print("GOAP Duel: Restarting...")
	get_tree().reload_current_scene()
