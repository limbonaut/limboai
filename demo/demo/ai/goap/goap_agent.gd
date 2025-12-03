## GOAP Demo Agent
## An agent that uses GOAP to plan and execute tactical decisions
extends CharacterBody2D

signal target_killed
signal health_changed(current: int, max_health: int)
signal ammo_changed(current: int)

const ATTACK_RANGE := 150.0  # Melee range
const SHOOTING_RANGE := 500.0  # Ranged weapon range
const PICKUP_RANGE := 80.0
const MOVE_SPEED := 300.0
const COVER_RANGE := 100.0  # Distance to be considered "at cover"

# World objects (set via inspector or found at runtime)
@export_node_path("Node2D") var target_path: NodePath
@export_node_path("Node2D") var weapon_pickup_path: NodePath
@export_node_path("Node2D") var ammo_pickup_path: NodePath
@export_node_path("Node2D") var health_pickup_path: NodePath
@export_node_path("Node2D") var cover_object_path: NodePath

# GOAP Goals for dynamic switching
@export var goal_kill_target: Resource  # GOAPGoal
@export var goal_avoid_damage: Resource  # GOAPGoal
@export var goal_regain_health: Resource  # GOAPGoal

var target: Node2D
var weapon_pickup: Node2D
var ammo_pickup: Node2D
var health_pickup: Node2D
var cover_object: Node2D  # Physical cover object (CoverObject class)

# LoS collision layer (same as CoverObject)
const LOS_COLLISION_LAYER := 16

# Agent state
var has_weapon := false
var ammo_count := 0  # Limited ammo for ranged attacks
var max_ammo := 10
var health := 100
var max_health := 100
var in_cover := false
var weapon_jammed := false
var _current_goal_type: String = "kill"  # "kill", "avoid", or "health"

# Weapon jam chance (15% by default)
@export var jam_chance := 0.15

@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var root: Node2D = $Root
@onready var bt_player: BTPlayer = $BTPlayer


func _ready() -> void:
	# Resolve node paths
	if not target_path.is_empty():
		target = get_node(target_path) as Node2D
	if not weapon_pickup_path.is_empty():
		weapon_pickup = get_node(weapon_pickup_path) as Node2D
	if not ammo_pickup_path.is_empty():
		ammo_pickup = get_node(ammo_pickup_path) as Node2D
	if not health_pickup_path.is_empty():
		health_pickup = get_node(health_pickup_path) as Node2D
	if not cover_object_path.is_empty():
		cover_object = get_node(cover_object_path) as Node2D
	# Connect to enemy attack signal for immediate threat response
	if target and target.has_signal("attack_started"):
		target.attack_started.connect(_on_threat_detected)
	# Initial sync - must run before first _physics_process
	_initial_sync()


func _initial_sync() -> void:
	# Initialize key blackboard variables that GOAP needs
	var bb := bt_player.get_blackboard()
	if bb:
		bb.set_var(&"under_threat", false)
		bb.set_var(&"enemy_attacking", false)
		bb.set_var(&"in_cover", false)
		bb.set_var(&"near_cover", false)
		bb.set_var(&"weapon_jammed", false)
		bb.set_var(&"low_health", false)
	_update_blackboard()
	health_changed.emit(health, max_health)
	ammo_changed.emit(ammo_count)


func _physics_process(_delta: float) -> void:
	_update_blackboard()
	move_and_slide()


func _update_blackboard() -> void:
	var bb := bt_player.get_blackboard()
	if not bb:
		return

	# Update proximity and availability checks for pickups
	var near_weapon_pickup := false
	var weapon_available := false
	if weapon_pickup and is_instance_valid(weapon_pickup):
		# Check if pickup is available (not collected and waiting to respawn)
		if weapon_pickup.has_method("is_available"):
			weapon_available = weapon_pickup.is_available()
		else:
			weapon_available = true  # Fallback for old pickups
		if weapon_available:
			near_weapon_pickup = global_position.distance_to(weapon_pickup.global_position) < PICKUP_RANGE

	var near_ammo := false
	var ammo_available := false
	if ammo_pickup and is_instance_valid(ammo_pickup):
		if ammo_pickup.has_method("is_available"):
			ammo_available = ammo_pickup.is_available()
		else:
			ammo_available = true
		if ammo_available:
			near_ammo = global_position.distance_to(ammo_pickup.global_position) < PICKUP_RANGE

	var near_health_pickup := false
	var health_available := false
	if health_pickup and is_instance_valid(health_pickup):
		if health_pickup.has_method("is_available"):
			health_available = health_pickup.is_available()
		else:
			health_available = true
		if health_available:
			near_health_pickup = global_position.distance_to(health_pickup.global_position) < PICKUP_RANGE

	var near_cover := false
	if cover_object and is_instance_valid(cover_object):
		near_cover = global_position.distance_to(cover_object.global_position) < COVER_RANGE

	var target_in_range := false
	var target_in_sight := false
	var target_visible := false
	if target and is_instance_valid(target):
		var dist := global_position.distance_to(target.global_position)
		target_in_range = dist < ATTACK_RANGE
		target_in_sight = dist < SHOOTING_RANGE  # Can shoot from this range
		# Check LoS - is there cover blocking the view?
		target_visible = has_los_to_target()

	# Check if target is dead
	var target_dead := false
	if target == null or not is_instance_valid(target):
		target_dead = true
	elif target.has_node("Health"):
		var health_node = target.get_node("Health")
		if health_node.has_method("get_current"):
			target_dead = health_node.get_current() <= 0

	# Check if enemy is preparing an attack (telegraphing)
	var enemy_attacking := false
	if target and is_instance_valid(target):
		if target.has_method("is_preparing_attack"):
			enemy_attacking = target.is_preparing_attack()

	# Derive states from variables
	var has_ammo := ammo_count > 0
	var weapon_loaded := has_weapon and has_ammo
	var low_health := health < 50
	var is_healthy := health >= 80
	# under_threat is true while enemy is telegraphing, regardless of cover
	# Being in cover protects from damage but the threat exists until attack fires
	var under_threat := enemy_attacking

	# Track state changes BEFORE updating blackboard (for goal switching)
	var prev_under_threat = bb.get_var(&"under_threat", false)
	var prev_low_health = bb.get_var(&"low_health", false)

	# Sync to blackboard
	bb.set_var(&"has_weapon", has_weapon)
	bb.set_var(&"has_ammo", has_ammo)
	bb.set_var(&"weapon_loaded", weapon_loaded)
	bb.set_var(&"ammo_count", ammo_count)
	bb.set_var(&"health", health)
	bb.set_var(&"low_health", low_health)
	bb.set_var(&"is_healthy", is_healthy)
	bb.set_var(&"in_cover", in_cover)
	bb.set_var(&"near_cover", near_cover)
	bb.set_var(&"target_in_range", target_in_range)
	bb.set_var(&"target_in_sight", target_in_sight)
	bb.set_var(&"target_visible", target_visible)
	bb.set_var(&"near_weapon_pickup", near_weapon_pickup)
	bb.set_var(&"near_ammo", near_ammo)
	bb.set_var(&"near_health_pickup", near_health_pickup)
	bb.set_var(&"weapon_available", weapon_available)
	bb.set_var(&"ammo_available", ammo_available)
	bb.set_var(&"health_available", health_available)
	bb.set_var(&"target_dead", target_dead)
	bb.set_var(&"enemy_attacking", enemy_attacking)
	bb.set_var(&"under_threat", under_threat)
	bb.set_var(&"weapon_jammed", weapon_jammed)
	bb.set_var(&"target", target)
	bb.set_var(&"weapon_pickup", weapon_pickup)
	bb.set_var(&"ammo_pickup", ammo_pickup)
	bb.set_var(&"health_pickup", health_pickup)
	bb.set_var(&"cover_object", cover_object)

	# Check for state changes and trigger goal re-evaluation
	if under_threat != prev_under_threat:
		print("GOAP STATE: under_threat changed from %s to %s" % [prev_under_threat, under_threat])
	if low_health != prev_low_health:
		print("GOAP STATE: low_health changed from %s to %s" % [prev_low_health, low_health])
	if under_threat != prev_under_threat or low_health != prev_low_health:
		_evaluate_goal(under_threat, low_health)


# Combat methods
func use_ammo() -> bool:
	# Can't use ammo if weapon is jammed
	if weapon_jammed:
		print("GOAP: Can't fire - weapon jammed!")
		return false

	if ammo_count > 0:
		# Check for random jam
		if randf() < jam_chance:
			weapon_jammed = true
			print("GOAP: WEAPON JAMMED! Need to unjam before firing.")
			return false

		ammo_count -= 1
		ammo_changed.emit(ammo_count)
		print("GOAP: Used ammo, remaining: %d" % ammo_count)
		return true
	return false


func unjam_weapon() -> void:
	weapon_jammed = false
	print("GOAP: Weapon unjammed!")


func add_ammo(amount: int) -> void:
	ammo_count = mini(ammo_count + amount, max_ammo)
	ammo_changed.emit(ammo_count)
	print("GOAP: Added ammo, total: %d" % ammo_count)


func take_damage(amount: int) -> void:
	var was_low_health := health < 50
	health = maxi(0, health - amount)
	health_changed.emit(health, max_health)
	print("GOAP: Took %d damage, health: %d" % [amount, health])
	if health <= 0:
		print("GOAP: Agent died!")
	# Check if we just became low health - trigger goal evaluation
	var is_low_health := health < 50
	if is_low_health and not was_low_health:
		# Get current threat state from blackboard
		var bb := bt_player.get_blackboard()
		var is_threatened := false
		if bb:
			is_threatened = bb.get_var(&"under_threat", false)
		print("GOAP STATE: low_health changed from false to true (via damage, under_threat=%s)" % is_threatened)
		_evaluate_goal(is_threatened, true)


func heal(amount: int) -> void:
	health = mini(health + amount, max_health)
	health_changed.emit(health, max_health)
	print("GOAP: Healed %d, health: %d" % [amount, health])


func enter_cover() -> void:
	in_cover = true
	print("GOAP: Entered cover")


func leave_cover() -> void:
	in_cover = false
	print("GOAP: Left cover")


# Movement methods compatible with demo agent pattern
func move(p_velocity: Vector2) -> void:
	velocity = lerp(velocity, p_velocity, 0.2)
	move_and_slide()
	update_facing()


func update_facing() -> void:
	if velocity.x > 10 and root.scale.x < 0:
		root.scale.x = 1.0
	elif velocity.x < -10 and root.scale.x > 0:
		root.scale.x = -1.0


func get_facing() -> float:
	return signf(root.scale.x)


## Checks if there is line-of-sight to the target (not blocked by cover)
func has_los_to_target() -> bool:
	if not target or not is_instance_valid(target):
		return false

	# Get physics space state for raycasting
	var space_state := get_world_2d().direct_space_state
	var query := PhysicsRayQueryParameters2D.create(global_position, target.global_position, LOS_COLLISION_LAYER)
	var result := space_state.intersect_ray(query)

	# If no collision, we have LoS
	return result.is_empty()


## Returns the position the agent should move to for cover against the target
func get_cover_position() -> Vector2:
	if not cover_object or not is_instance_valid(cover_object):
		return global_position

	if cover_object.has_method("get_cover_position_against"):
		if target and is_instance_valid(target):
			return cover_object.get_cover_position_against(target.global_position)

	# Fallback: just use cover object position
	return cover_object.global_position


## Called when enemy starts attacking - triggers immediate replan for cover-seeking
func _on_threat_detected() -> void:
	print("GOAP: Threat detected! Goal switching handled by _update_blackboard.")


## Evaluates which goal should be active based on current state
## Priority: AvoidDamage (under threat) > RegainHealth (low health) > KillTarget (default)
func _evaluate_goal(is_threatened: bool, is_low_health: bool) -> void:
	if not bt_player:
		return

	var bt_instance = bt_player.get_bt_instance()
	if not bt_instance:
		return

	var root_task = bt_instance.get_root_task()
	if not root_task:
		return

	# Find the BTRunGOAPPlan task
	var goap_task = _find_goap_task(root_task)
	if not goap_task:
		return

	# Determine desired goal based on priority
	var new_goal_type: String
	var new_goal: Resource
	if is_threatened:
		new_goal_type = "avoid"
		new_goal = goal_avoid_damage
	elif is_low_health:
		new_goal_type = "health"
		new_goal = goal_regain_health
	else:
		new_goal_type = "kill"
		new_goal = goal_kill_target

	if new_goal_type == _current_goal_type:
		return  # No change needed

	if not new_goal:
		push_warning("GOAP: Goal resource not set for type: " + new_goal_type)
		return

	print("GOAP: Switching goal to %s" % new_goal.goal_name)
	goap_task.set_goal(new_goal)
	goap_task.interrupt()  # Force replan with new goal
	_current_goal_type = new_goal_type


## Recursively finds the BTRunGOAPPlan task in the behavior tree
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
