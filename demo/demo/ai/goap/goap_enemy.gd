## GOAP Demo Enemy
## A target that periodically attacks with a telegraphed warning
extends CharacterBody2D

signal attack_started  # Emitted when telegraph begins
signal attack_fired    # Emitted when actual attack happens
signal attack_ended    # Emitted when attack cycle ends

@export var attack_interval := 8.0  # Seconds between attacks
@export var telegraph_duration := 2.5  # Warning time before attack
@export var attack_damage := 25

var is_telegraphing := false
var is_attacking := false
var attack_timer := 0.0
var telegraph_timer := 0.0
var target_agent: Node2D = null

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
	telegraph_timer = telegraph_duration

	# Show visual warning
	if telegraph_indicator:
		telegraph_indicator.visible = true
		_animate_telegraph()

	print("ENEMY: Preparing to attack! (%.1fs warning)" % telegraph_duration)
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

	print("ENEMY: Firing attack!")
	attack_fired.emit()

	# Deal damage to target if not in cover
	if target_agent and is_instance_valid(target_agent):
		if target_agent.has_method("is_in_cover") and target_agent.is_in_cover():
			print("ENEMY: Target is in cover - attack blocked!")
		elif target_agent.get("in_cover") == true:
			print("ENEMY: Target is in cover - attack blocked!")
		else:
			if target_agent.has_method("take_damage"):
				target_agent.take_damage(attack_damage)
				print("ENEMY: Hit target for %d damage!" % attack_damage)

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
