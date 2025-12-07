## Combat Component
## Handles health, ammo, weapon state and combat mechanics
class_name CombatComponent
extends Node

const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

enum WeaponType { NONE, MELEE, RANGED }
enum CombatMode { MELEE, RANGED }

signal health_changed(current: int, max_health: int)
signal ammo_changed(current: int)
signal weapon_jammed
signal weapon_unjammed
signal suppression_changed(is_suppressed: bool)
signal weapon_equipped(weapon_type: int)
signal died
signal speed_boost_changed(is_boosted: bool)

@export var max_health: int = GOAPConfigClass.DEFAULT_MAX_HEALTH
@export var max_ammo: int = GOAPConfigClass.DEFAULT_MAX_AMMO
@export var jam_chance: float = GOAPConfigClass.DEFAULT_JAM_CHANCE

## Current weapon type equipped by the agent
var weapon_type: WeaponType = WeaponType.NONE:
	get:
		return weapon_type
	set(value):
		var old_type := weapon_type
		weapon_type = value
		if old_type != value and value != WeaponType.NONE:
			weapon_equipped.emit(value)
			print("GOAP Combat: Weapon equipped - %s" % WeaponType.keys()[value])

## Preferred combat mode - what weapon type agent prefers (set by demo)
var preferred_mode: CombatMode = CombatMode.RANGED

## Accuracy modifier (1.0 = perfect, 0.5 = 50% hit chance)
var accuracy_modifier: float = 1.0

## Suppression state - when suppressed, agent can't leave cover
var is_suppressed: bool = false
var _suppression_timer: float = 0.0

## Speed boost state - when boosted, agent moves faster
var is_speed_boosted: bool = false
var _speed_boost_timer: float = 0.0

var health: int:
	get:
		return _health
	set(value):
		_health = clampi(value, 0, max_health)
		health_changed.emit(_health, max_health)

var ammo_count: int:
	get:
		return _ammo_count
	set(value):
		_ammo_count = clampi(value, 0, max_ammo)
		ammo_changed.emit(_ammo_count)

var has_weapon := false
var weapon_jammed_state := false

var _health: int
var _ammo_count: int


func _ready() -> void:
	_health = max_health
	_ammo_count = 0


## Attempts to use one ammo. Returns true if successful.
## May trigger weapon jam.
func use_ammo() -> bool:
	if weapon_jammed_state:
		print("GOAP Combat: Can't fire - weapon jammed!")
		return false

	if _ammo_count <= 0:
		return false

	# Check for random jam
	if randf() < jam_chance:
		weapon_jammed_state = true
		weapon_jammed.emit()
		print("GOAP Combat: WEAPON JAMMED! Need to unjam before firing.")
		return false

	_ammo_count -= 1
	ammo_changed.emit(_ammo_count)
	print("GOAP Combat: Used ammo, remaining: %d" % _ammo_count)
	return true


## Clears weapon jam
func unjam_weapon() -> void:
	weapon_jammed_state = false
	weapon_unjammed.emit()
	print("GOAP Combat: Weapon unjammed!")


## Adds ammo up to max capacity
func add_ammo(amount: int) -> void:
	_ammo_count = mini(_ammo_count + amount, max_ammo)
	ammo_changed.emit(_ammo_count)
	print("GOAP Combat: Added ammo, total: %d" % _ammo_count)


## Takes damage and returns true if still alive
func take_damage(amount: int) -> bool:
	var was_low_health := is_low_health()
	_health = maxi(0, _health - amount)
	health_changed.emit(_health, max_health)
	print("GOAP Combat: Took %d damage, health: %d" % [amount, _health])

	if _health <= 0:
		died.emit()
		print("GOAP Combat: Died!")
		return false

	return true


## Heals up to max health
func heal(amount: int) -> void:
	_health = mini(_health + amount, max_health)
	health_changed.emit(_health, max_health)
	print("GOAP Combat: Healed %d, health: %d" % [amount, _health])


## Returns true if health is below low threshold
func is_low_health() -> bool:
	return _health < GOAPConfigClass.LOW_HEALTH_THRESHOLD


## Returns true if health is at or above healthy threshold
func is_healthy() -> bool:
	return _health >= GOAPConfigClass.HEALTHY_THRESHOLD


## Returns true if weapon is loaded and ready to fire
func is_weapon_ready() -> bool:
	return has_weapon and _ammo_count > 0 and not weapon_jammed_state


## Forces a health changed emission (used when external systems modify health)
func _emit_health_changed() -> void:
	health_changed.emit(_health, max_health)


## Returns true if wielding a melee weapon
func is_melee() -> bool:
	return weapon_type == WeaponType.MELEE


## Returns true if wielding a ranged weapon
func is_ranged() -> bool:
	return weapon_type == WeaponType.RANGED


## Returns true if shot should hit based on accuracy modifier
func should_hit() -> bool:
	return randf() < accuracy_modifier


## Returns movement speed based on weapon type (ranged is faster for kiting)
## Speed boost is applied on top of weapon-based speed
func get_move_speed() -> float:
	var base_speed: float
	match weapon_type:
		WeaponType.RANGED:
			base_speed = GOAPConfigClass.RANGED_MOVE_SPEED
		WeaponType.MELEE:
			base_speed = GOAPConfigClass.MELEE_MOVE_SPEED
		_:
			base_speed = GOAPConfigClass.MOVE_SPEED

	if is_speed_boosted:
		return base_speed * GOAPConfigClass.SPEED_BOOST_MULTIPLIER
	return base_speed


## Apply suppression effect to the agent
func apply_suppression() -> void:
	var was_suppressed := is_suppressed
	is_suppressed = true
	_suppression_timer = GOAPConfigClass.SUPPRESSION_DURATION
	if not was_suppressed:
		suppression_changed.emit(true)
		print("GOAP Combat: SUPPRESSED! Can't leave cover for %.1fs" % _suppression_timer)


## Apply speed boost effect to the agent
func apply_speed_boost() -> void:
	var was_boosted := is_speed_boosted
	is_speed_boosted = true
	_speed_boost_timer = GOAPConfigClass.SPEED_BOOST_DURATION
	if not was_boosted:
		speed_boost_changed.emit(true)
		print("GOAP Combat: SPEED BOOST! Moving %.0f%% faster for %.1fs" % [(GOAPConfigClass.SPEED_BOOST_MULTIPLIER - 1.0) * 100, _speed_boost_timer])


## Called every frame to update timers
func _process(delta: float) -> void:
	# Update suppression timer
	if _suppression_timer > 0.0:
		_suppression_timer -= delta
		if _suppression_timer <= 0.0:
			is_suppressed = false
			suppression_changed.emit(false)
			print("GOAP Combat: Suppression ended")

	# Update speed boost timer
	if _speed_boost_timer > 0.0:
		_speed_boost_timer -= delta
		if _speed_boost_timer <= 0.0:
			is_speed_boosted = false
			speed_boost_changed.emit(false)
			print("GOAP Combat: Speed boost ended")
