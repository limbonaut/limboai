## GOAP Demo Manager
## Manages the GOAP demonstration scene with plan visualization
extends Node2D

@onready var plan_label: Label = %PlanLabel
@onready var goal_display: Label = %GoalDisplay
@onready var plan_chain_label: RichTextLabel = %PlanChainLabel
@onready var scarcity_toggle: CheckButton = %ScarcityToggle
@onready var goap_agent: CharacterBody2D = $GOAPAgent
@onready var target: Node2D = $Target
@onready var weapon_pickup: Node2D = $WeaponPickup
@onready var ammo_pickup: Node2D = $AmmoPickup
@onready var health_pickup: Node2D = $HealthPickup
@onready var cover_object: Node2D = $CoverObject

var initial_positions := {}

# Plan tracking for visualization
var _prev_plan_hash: int = 0
var _prev_goal_name: String = ""
var _prev_action_index: int = -1


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
	_update_plan_chain_display()

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
		goal_display.text = "No BTPlayer"
		return

	var bb := bt_player.get_blackboard()
	if not bb:
		plan_label.text = "Blackboard not found"
		goal_display.text = "No Blackboard"
		return

	var lines := []

	# Get goal name and update large centered display
	var goal_name := _get_current_goal_name(bt_player)
	goal_display.text = "Goal: %s" % goal_name

	# Enemy attack info
	lines.append("")
	lines.append("--- ENEMY ---")
	if target and is_instance_valid(target):
		if target.is_telegraphing:
			lines.append("Attacking: PIERCE (%.1fs)" % target.telegraph_timer)
		else:
			lines.append("Next attack: %.1fs" % target.attack_timer)

	# Combat status
	lines.append("")
	lines.append("--- COMBAT ---")
	lines.append("Health: %d/%d" % [goap_agent.health, goap_agent.max_health])
	lines.append("Ammo: %d/%d" % [goap_agent.ammo_count, goap_agent.max_ammo])
	lines.append("has_weapon: %s" % _bool_str(bb.get_var(&"has_weapon", false)))
	lines.append("weapon_loaded: %s" % _bool_str(bb.get_var(&"weapon_loaded", false)))

	# Tactical status
	lines.append("")
	lines.append("--- TACTICAL ---")
	lines.append("in_cover: %s" % _bool_str(bb.get_var(&"in_cover", false)))
	lines.append("under_threat: %s" % _bool_str(bb.get_var(&"under_threat", false)))
	lines.append("low_health: %s" % _bool_str(bb.get_var(&"low_health", false)))

	# Proximity status
	lines.append("")
	lines.append("--- PROXIMITY ---")
	lines.append("target_in_sight: %s" % _bool_str(bb.get_var(&"target_in_sight", false)))
	lines.append("target_in_range: %s" % _bool_str(bb.get_var(&"target_in_range", false)))
	lines.append("near_cover: %s" % _bool_str(bb.get_var(&"near_cover", false)))
	lines.append("near_health: %s" % _bool_str(bb.get_var(&"near_health_pickup", false)))

	plan_label.text = "\n".join(lines)


func _get_current_goal_name(bt_player: BTPlayer) -> String:
	var bt_instance = bt_player.get_bt_instance()
	if not bt_instance:
		return "(no instance)"

	var root_task = bt_instance.get_root_task()
	if not root_task:
		return "(no root task)"

	# Find BTRunGOAPPlan task - it's likely a child of root
	var goap_task = _find_goap_task(root_task)
	if not goap_task:
		return "(no GOAP task)"

	# Get the current goal from the GOAP task
	if goap_task.has_method("get_goal"):
		var goal = goap_task.get_goal()
		if goal:
			return goal.goal_name
		return "(planning...)"

	return "(unknown)"


func _find_goap_task(task) -> Variant:
	# Check if this task is BTRunGOAPPlan
	if task.get_class() == "BTRunGOAPPlan":
		return task

	# Check children
	var child_count = task.get_child_count()
	for i in range(child_count):
		var child = task.get_child(i)
		var result = _find_goap_task(child)
		if result:
			return result

	return null


func _bool_str(value: bool) -> String:
	return "YES" if value else "no"


func _on_restart_pressed() -> void:
	print("GOAP Demo: Restarting...")
	get_tree().reload_current_scene()


## Updates the plan chain visualization showing current action sequence
func _update_plan_chain_display() -> void:
	var bt_player: BTPlayer = goap_agent.get_node_or_null("BTPlayer")
	if not bt_player:
		plan_chain_label.text = "[color=gray]No BTPlayer[/color]"
		return

	var goap_task = _find_goap_task_from_player(bt_player)
	if not goap_task:
		plan_chain_label.text = "[color=gray]No GOAP task found[/color]"
		return

	var plan = goap_task.get_current_plan()
	var current_idx = goap_task.get_current_action_index()

	var goal = goap_task.get_goal()
	var goal_name = goal.goal_name if goal else "?"

	if plan.is_empty():
		# Debug: print why there's no plan (reset on goal change)
		var bb := bt_player.get_blackboard()
		if bb and (_prev_plan_hash != -999 or _prev_goal_name != goal_name):
			_prev_plan_hash = -999
			_prev_goal_name = goal_name
			print("GOAP DEBUG: No plan for goal '%s'. World state:" % goal_name)
			print("  has_weapon=%s, weapon_loaded=%s, weapon_jammed=%s" % [
				bb.get_var(&"has_weapon", false),
				bb.get_var(&"weapon_loaded", false),
				bb.get_var(&"weapon_jammed", false)])
			print("  in_cover=%s, under_threat=%s, target_visible=%s" % [
				bb.get_var(&"in_cover", false),
				bb.get_var(&"under_threat", false),
				bb.get_var(&"target_visible", false)])
			print("  ammo_available=%s, weapon_available=%s" % [
				bb.get_var(&"ammo_available", false),
				bb.get_var(&"weapon_available", false)])
			print("  target_in_sight=%s, target_in_range=%s" % [
				bb.get_var(&"target_in_sight", false),
				bb.get_var(&"target_in_range", false)])
		plan_chain_label.text = "[color=gray]No active plan[/color]"
		return

	# Build vertical plan display with BBCode colors
	var lines: Array[String] = []
	for i in range(plan.size()):
		var action = plan[i]
		var action_name: String = action.action_name
		var prefix = "  "
		if i < current_idx:
			# Completed action - green with checkmark
			lines.append("[color=#4a4]✓ %s[/color]" % action_name)
		elif i == current_idx:
			# Current action - yellow, highlighted with arrow
			lines.append("[color=#ff0]► %s[/color]" % action_name)
		else:
			# Pending action - gray with dot
			lines.append("[color=#aaa]○ %s[/color]" % action_name)

	lines.append("[color=#888]  ↓[/color]")
	lines.append("[color=#0f0]★ GOAL: %s[/color]" % goal_name)

	plan_chain_label.text = "\n".join(lines)


## Helper to find GOAP task from BTPlayer
func _find_goap_task_from_player(bt_player: BTPlayer) -> Variant:
	var bt_instance = bt_player.get_bt_instance()
	if not bt_instance:
		return null
	var root_task = bt_instance.get_root_task()
	if not root_task:
		return null
	return _find_goap_task(root_task)


## Scarcity mode toggle handler
func _on_scarcity_toggled(enabled: bool) -> void:
	if enabled:
		goap_agent.max_ammo = 3
		goap_agent.ammo_count = mini(goap_agent.ammo_count, 3)
		ammo_pickup.respawn_time = 12.0
		health_pickup.respawn_time = 15.0
		print("GOAP Demo: Scarcity mode ON - ammo limited to 3")
	else:
		goap_agent.max_ammo = 10
		ammo_pickup.respawn_time = 5.0
		health_pickup.respawn_time = 8.0
		print("GOAP Demo: Scarcity mode OFF - normal resources")
