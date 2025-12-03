## GOAP Demo Enemy
## A target that periodically attacks with a telegraphed warning
extends CharacterBody2D

signal attack_started  # Emitted when telegraph begins
signal attack_fired    # Emitted when actual attack happens
signal attack_ended    # Emitted when attack cycle ends

@export var attack_interval := 8.0  # Seconds between attacks
@export var telegraph_duration := 3.5  # Warning time before attack (increased for AI reaction time)
@export var pierce_damage := 50  # Damage when target NOT in cover
@export var pierce_cover_damage := 25  # Damage when target IS in cover

# Variable telegraph settings - creates natural damage opportunities
@export var min_telegraph := 1.2  # Sometimes too fast to reach cover
@export var max_telegraph := 3.5  # Easy to reach cover

var is_telegraphing := false
var is_attacking := false
var attack_timer := 0.0
var telegraph_timer := 0.0
var target_agent: Node2D = null
var current_telegraph_duration := 3.5  # Actual duration for current attack

@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var root: Node2D = $Root
@onready var telegraph_indicator: Node2D = $TelegraphIndicator


func _ready() -> void:
	attack_timer = 1.5  # Start first attack quickly for demo visibility
	if telegraph_indicator:
		telegraph_indicator.visible = false


func _process(delta: float) -> void:
	if is_telegraphing:
		telegraph_timer -= delta
		if telegraph_timer <= 0:
			_fire_attack()
	else:
		attack_timer -= delta
		if attack_timer <= 0:
			_start_telegraph()


func _start_telegraph() -> void:
	is_telegraphing = true

	# Randomize telegraph duration for this attack
	current_telegraph_duration = randf_range(min_telegraph, max_telegraph)
	telegraph_timer = current_telegraph_duration

	# Show visual warning - all attacks are pierce attacks now
	if telegraph_indicator:
		telegraph_indicator.visible = true
		_animate_telegraph()
		var label = telegraph_indicator.get_node_or_null("Label")
		if label:
			label.text = "! PIERCE !"
			label.modulate = Color(1.0, 0.5, 0.0)  # Orange for pierce

	print("ENEMY: Preparing PIERCE attack! (%.1fs warning)" % current_telegraph_duration)
	attack_started.emit()


func _animate_telegraph() -> void:
	# Pulse the telegraph indicator
	var tween = create_tween()
	tween.set_loops(int(telegraph_duration / 0.3))
	tween.tween_property(telegraph_indicator, "modulate:a", 0.3, 0.15)
	tween.tween_property(telegraph_indicator, "modulate:a", 1.0, 0.15)


func _fire_attack() -> void:
	is_telegraphing = false
	is_attacking = true

	if telegraph_indicator:
		telegraph_indicator.visible = false

	print("ENEMY: Firing PIERCE attack!")
	attack_fired.emit()

	# All attacks are pierce attacks - deal variable damage based on cover
	if target_agent and is_instance_valid(target_agent):
		var target_in_cover := false
		if target_agent.has_method("is_in_cover"):
			target_in_cover = target_agent.is_in_cover()
		elif target_agent.get("in_cover") == true:
			target_in_cover = true

		if target_agent.has_method("take_damage"):
			var damage := pierce_cover_damage if target_in_cover else pierce_damage
			target_agent.take_damage(damage)
			if target_in_cover:
				print("ENEMY: Target in cover - reduced damage! Hit for %d damage!" % damage)
			else:
				print("ENEMY: Hit target for %d damage!" % damage)

	# Reset attack cycle
	attack_timer = attack_interval
	is_attacking = false
	attack_ended.emit()


func set_target(p_target: Node2D) -> void:
	target_agent = p_target


func is_preparing_attack() -> bool:
	return is_telegraphing


func get_time_until_attack() -> float:
	if is_telegraphing:
		return telegraph_timer
	return -1.0


# Compatibility with existing dummy
func _on_health_damaged(_amount: float, _knockback: Vector2) -> void:
	if has_node("Hurtbox"):
		var hurtbox = get_node("Hurtbox")
		if hurtbox.get("last_attack_vector"):
			root.scale.x = -signf(hurtbox.last_attack_vector.x)
	animation_player.clear_queue()
	animation_player.play(&"hurt", 0.1)


func get_facing() -> float:
	return signf(root.scale.x)
