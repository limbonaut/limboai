extends Node2D

@onready var agent: CharacterBody2D = $Agent
@onready var agent_2: CharacterBody2D = $Agent2


func _ready() -> void:
	var waypoints: Array[Node] = $Waypoints.get_children()

	for wp in waypoints:
		agent.add_waypoint(wp.global_position)

	waypoints.reverse()
	for wp in waypoints:
		agent_2.add_waypoint(wp.global_position)


