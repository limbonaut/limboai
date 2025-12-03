## GOAP Demo Manager
## Manages the GOAP demonstration scene
extends Node2D

@onready var plan_label: Label = %PlanLabel
@onready var goap_agent: CharacterBody2D = $GOAPAgent
@onready var target: Node2D = $Target
@onready var weapon_pickup: Node2D = $WeaponPickup
@onready var ammo_pickup: Node2D = $AmmoPickup
@onready var health_pickup: Node2D = $HealthPickup
@onready var cover_object: Node2D = $CoverObject

var initial_positions := {}


func _ready() -> void:
	# Store initial positions for restart
	initial_positions["agent"] = goap_agent.position
	initial_positions["target"] = target.position
	initial_positions["weapon"] = weapon_pickup.position
	initial_positions["ammo"] = ammo_pickup.position
	initial_positions["health"] = health_pickup.position
	initial_positions["cover"] = cover_object.position

	# Connect enemy to agent for attack targeting
	if target.has_method("set_target"):
		target.set_target(goap_agent)

	print("GOAP Demo: Full Tactical System Ready!")


var _perf_timer := 0.0

func _process(delta: float) -> void:
	_update_plan_display()

	# Performance diagnostics - log every 2 seconds
	_perf_timer += delta
	if _perf_timer >= 2.0:
		_perf_timer = 0.0
		var star_count := get_tree().get_nodes_in_group("projectiles").size() if is_inside_tree() else 0
		var node_count := Performance.get_monitor(Performance.OBJECT_NODE_COUNT)
		var fps := Engine.get_frames_per_second()
		var physics_time := Performance.get_monitor(Performance.TIME_PHYSICS_PROCESS) * 1000
		var render_time := Performance.get_monitor(Performance.TIME_PROCESS) * 1000
		print("PERF: FPS=%d  Nodes=%d  Stars=%d  Physics=%.1fms  Process=%.1fms" % [fps, node_count, star_count, physics_time, render_time])


func _update_plan_display() -> void:
	var bt_player: BTPlayer = goap_agent.get_node_or_null("BTPlayer")
	if not bt_player:
		plan_label.text = "BTPlayer not found"
		return

	var bb := bt_player.get_blackboard()
	if not bb:
		plan_label.text = "Blackboard not found"
		return

	var lines := []

	# Combat status
	lines.append("--- COMBAT ---")
	lines.append("Health: %d/%d" % [goap_agent.health, goap_agent.max_health])
	lines.append("Ammo: %d/%d" % [goap_agent.ammo_count, goap_agent.max_ammo])
	lines.append("has_weapon: %s" % _bool_str(bb.get_var(&"has_weapon", false)))
	lines.append("weapon_loaded: %s" % _bool_str(bb.get_var(&"weapon_loaded", false)))
	lines.append("has_ammo: %s" % _bool_str(bb.get_var(&"has_ammo", false)))

	# Tactical status
	lines.append("")
	lines.append("--- TACTICAL ---")
	lines.append("in_cover: %s" % _bool_str(bb.get_var(&"in_cover", false)))
	lines.append("enemy_attacking: %s" % _bool_str(bb.get_var(&"enemy_attacking", false)))
	lines.append("under_threat: %s" % _bool_str(bb.get_var(&"under_threat", false)))
	lines.append("low_health: %s" % _bool_str(bb.get_var(&"low_health", false)))

	# Proximity status
	lines.append("")
	lines.append("--- PROXIMITY ---")
	lines.append("target_in_sight: %s" % _bool_str(bb.get_var(&"target_in_sight", false)))
	lines.append("target_in_range: %s" % _bool_str(bb.get_var(&"target_in_range", false)))
	lines.append("near_weapon: %s" % _bool_str(bb.get_var(&"near_weapon_pickup", false)))
	lines.append("near_ammo: %s" % _bool_str(bb.get_var(&"near_ammo", false)))
	lines.append("near_cover: %s" % _bool_str(bb.get_var(&"near_cover", false)))
	lines.append("near_health: %s" % _bool_str(bb.get_var(&"near_health_pickup", false)))

	# Goal status
	lines.append("")
	lines.append("--- GOAL ---")
	lines.append("target_dead: %s" % _bool_str(bb.get_var(&"target_dead", false)))

	plan_label.text = "\n".join(lines)


func _bool_str(value: bool) -> String:
	return "YES" if value else "no"


func _on_restart_pressed() -> void:
	print("GOAP Demo: Restarting...")
	get_tree().reload_current_scene()
