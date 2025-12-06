## Battle Simulator
## Runs GOAP agent battles for evolutionary optimization.
## Can run in accelerated mode for fast fitness evaluation.
##
## Features:
## - Configurable time scale for fast simulation
## - Tracks damage dealt/taken, survival time, wins/losses
## - Applies WeightGenome configurations to agents
## - Supports 1v1 battles between different genome configurations
class_name BattleSimulator
extends Node

const WeightGenomeClass = preload("res://demo/ai/goap/optimization/weight_genome.gd")
const GOAPConfigClass = preload("res://demo/ai/goap/goap_config.gd")

signal battle_completed(result: BattleResult)
signal simulation_progress(current: int, total: int)

## Battle result data
class BattleResult:
	var winner_genome: Resource  # null for draw
	var loser_genome: Resource   # null for draw
	var winner_side: String  # "red", "blue", or "draw"
	var duration: float
	var red_damage_dealt: float
	var red_damage_taken: float
	var blue_damage_dealt: float
	var blue_damage_taken: float
	var red_final_health: int
	var blue_final_health: int

	func _init() -> void:
		winner_side = "draw"
		duration = 0.0
		red_damage_dealt = 0.0
		red_damage_taken = 0.0
		blue_damage_dealt = 0.0
		blue_damage_taken = 0.0
		red_final_health = 0
		blue_final_health = 0

## Configuration
@export var time_scale := 4.0  # Run simulation faster
@export var max_battle_duration := 60.0  # Max seconds before declaring draw
@export var battle_scene_path := "res://demo/ai/goap/goap_duel_demo.tscn"

## State tracking
var _current_battle: Node = null
var _red_genome: Resource = null
var _blue_genome: Resource = null
var _battle_time := 0.0
var _red_initial_health := 100
var _blue_initial_health := 100
var _red_damage_dealt := 0.0
var _blue_damage_dealt := 0.0
var _is_battle_running := false

## References to agents in battle
var _red_agent: CharacterBody2D = null
var _blue_agent: CharacterBody2D = null


func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS


func _process(delta: float) -> void:
	if not _is_battle_running:
		return

	_battle_time += delta

	# Check for timeout
	if _battle_time >= max_battle_duration:
		_end_battle_timeout()


## Starts a battle between two genome configurations
func start_battle(red_genome: Resource, blue_genome: Resource) -> void:
	_red_genome = red_genome
	_blue_genome = blue_genome
	_battle_time = 0.0
	_red_damage_dealt = 0.0
	_blue_damage_dealt = 0.0
	_is_battle_running = true

	# Load battle scene
	var scene := load(battle_scene_path) as PackedScene
	if not scene:
		push_error("BattleSimulator: Could not load battle scene: " + battle_scene_path)
		return

	_current_battle = scene.instantiate()
	add_child(_current_battle)

	# Set time scale for faster simulation
	Engine.time_scale = time_scale

	# Wait for scene to be ready, then configure
	await get_tree().process_frame
	await get_tree().process_frame

	_configure_battle()


## Configures battle with genome weights
func _configure_battle() -> void:
	if not _current_battle:
		return

	# Find agents
	_red_agent = _current_battle.get_node_or_null("AgentRed")
	_blue_agent = _current_battle.get_node_or_null("AgentBlue")

	if not _red_agent or not _blue_agent:
		push_error("BattleSimulator: Could not find agents in battle scene")
		_cleanup_battle()
		return

	# Store initial health
	_red_initial_health = _red_agent.max_health
	_blue_initial_health = _blue_agent.max_health

	# Apply genome weights to agents
	_apply_genome_to_agent(_red_agent, _red_genome)
	_apply_genome_to_agent(_blue_agent, _blue_genome)

	# Connect death signals
	_red_agent.died.connect(_on_red_died)
	_blue_agent.died.connect(_on_blue_died)

	# Connect damage signals for tracking
	_red_agent.health_changed.connect(_on_red_health_changed)
	_blue_agent.health_changed.connect(_on_blue_health_changed)

	# Hide UI elements if present
	_hide_ui()


## Applies a WeightGenome to an agent
func _apply_genome_to_agent(agent: CharacterBody2D, genome: Resource) -> void:
	if not genome:
		return

	# Apply to GoalEvaluator
	var goal_evaluator = agent.get_node_or_null("GoalEvaluator")
	if goal_evaluator:
		# Set timing constants via a helper method we'll add
		_apply_timing_weights(goal_evaluator, genome)

	# Apply to CombatComponent
	var combat = agent.get_node_or_null("CombatComponent")
	if combat:
		_apply_combat_weights(combat, genome)

	# Apply to WorldStateManager (for thresholds)
	var world_state = agent.get_node_or_null("WorldStateManager")
	if world_state:
		_apply_threshold_weights(world_state, genome)

	# Store genome reference on agent for action cost lookups
	agent.set_meta("weight_genome", genome)


## Applies timing weights to goal evaluator
func _apply_timing_weights(goal_evaluator: Node, genome: Resource) -> void:
	# These are constants in the original, but we can override via meta
	goal_evaluator.set_meta("goal_switch_cooldown", genome.goal_switch_cooldown)
	goal_evaluator.set_meta("defensive_goal_commitment", genome.defensive_goal_commitment)
	goal_evaluator.set_meta("attack_goal_commitment", genome.attack_goal_commitment)
	goal_evaluator.set_meta("defensive_timeout", genome.defensive_timeout)


## Applies combat weights
func _apply_combat_weights(combat: Node, genome: Resource) -> void:
	# Movement speeds are typically on the movement component
	var agent := combat.get_parent()
	var movement = agent.get_node_or_null("MovementComponent")
	if movement:
		movement.set_meta("ranged_move_speed", genome.ranged_move_speed)
		movement.set_meta("melee_move_speed", genome.melee_move_speed)


## Applies threshold weights
func _apply_threshold_weights(world_state: Node, genome: Resource) -> void:
	world_state.set_meta("low_health_threshold", genome.low_health_threshold)
	world_state.set_meta("healthy_threshold", genome.healthy_threshold)
	world_state.set_meta("retreat_distance", genome.retreat_distance)
	world_state.set_meta("close_gap_threshold", genome.close_gap_threshold)
	world_state.set_meta("too_close_threshold", genome.too_close_threshold)


## Hides UI for headless mode
func _hide_ui() -> void:
	if not _current_battle:
		return

	# Find and hide UI nodes
	var ui := _current_battle.get_node_or_null("UI")
	if ui:
		ui.visible = false

	var winner_label := _current_battle.get_node_or_null("%WinnerLabel")
	if winner_label:
		winner_label.visible = false


var _last_red_health := 100
var _last_blue_health := 100


func _on_red_health_changed(current: int, max_hp: int) -> void:
	if current < _last_red_health:
		_blue_damage_dealt += _last_red_health - current
	_last_red_health = current


func _on_blue_health_changed(current: int, max_hp: int) -> void:
	if current < _last_blue_health:
		_red_damage_dealt += _last_blue_health - current
	_last_blue_health = current


func _on_red_died() -> void:
	_end_battle("blue")


func _on_blue_died() -> void:
	_end_battle("red")


func _end_battle_timeout() -> void:
	# Determine winner by remaining health percentage
	var red_health_pct := float(_red_agent.health) / float(_red_initial_health) if _red_agent else 0.0
	var blue_health_pct := float(_blue_agent.health) / float(_blue_initial_health) if _blue_agent else 0.0

	if red_health_pct > blue_health_pct + 0.1:  # Need 10% advantage to win
		_end_battle("red")
	elif blue_health_pct > red_health_pct + 0.1:
		_end_battle("blue")
	else:
		_end_battle("draw")


func _end_battle(winner: String) -> void:
	_is_battle_running = false
	Engine.time_scale = 1.0

	# Build result
	var result := BattleResult.new()
	result.winner_side = winner
	result.duration = _battle_time
	result.red_damage_dealt = _red_damage_dealt
	result.red_damage_taken = _blue_damage_dealt
	result.blue_damage_dealt = _blue_damage_dealt
	result.blue_damage_taken = _red_damage_dealt
	result.red_final_health = _red_agent.health if _red_agent else 0
	result.blue_final_health = _blue_agent.health if _blue_agent else 0

	match winner:
		"red":
			result.winner_genome = _red_genome
			result.loser_genome = _blue_genome
		"blue":
			result.winner_genome = _blue_genome
			result.loser_genome = _red_genome
		"draw":
			result.winner_genome = null
			result.loser_genome = null

	# Update genome fitness tracking
	_update_genome_stats(result)

	# Cleanup
	_cleanup_battle()

	# Emit result
	battle_completed.emit(result)


func _update_genome_stats(result: BattleResult) -> void:
	# Update red genome stats
	if _red_genome:
		_red_genome.damage_dealt += result.red_damage_dealt
		_red_genome.damage_taken += result.red_damage_taken
		_red_genome.survival_time += result.duration
		if result.winner_side == "red":
			_red_genome.wins += 1
		elif result.winner_side == "blue":
			_red_genome.losses += 1

	# Update blue genome stats
	if _blue_genome:
		_blue_genome.damage_dealt += result.blue_damage_dealt
		_blue_genome.damage_taken += result.blue_damage_taken
		_blue_genome.survival_time += result.duration
		if result.winner_side == "blue":
			_blue_genome.wins += 1
		elif result.winner_side == "red":
			_blue_genome.losses += 1


func _cleanup_battle() -> void:
	if _current_battle:
		_current_battle.queue_free()
		_current_battle = null

	_red_agent = null
	_blue_agent = null
	_red_genome = null
	_blue_genome = null
	_last_red_health = 100
	_last_blue_health = 100


## Runs a tournament between all genomes in a population
## Each genome fights every other genome once
func run_tournament(pop: Array) -> void:
	var total_battles := pop.size() * (pop.size() - 1) / 2
	var battles_completed := 0

	for i in range(pop.size()):
		for j in range(i + 1, pop.size()):
			await run_single_battle(pop[i], pop[j])
			battles_completed += 1
			simulation_progress.emit(battles_completed, total_battles)


## Runs a single battle and waits for completion
func run_single_battle(genome_a: Resource, genome_b: Resource) -> BattleResult:
	# Randomly assign red/blue to avoid positional bias
	var red: Resource
	var blue: Resource
	if randf() < 0.5:
		red = genome_a
		blue = genome_b
	else:
		red = genome_b
		blue = genome_a

	start_battle(red, blue)

	# Wait for battle to complete
	var result: BattleResult = await battle_completed
	return result


## Quick test function - runs a single battle with default genomes
func test_battle() -> void:
	var genome_a = WeightGenomeClass.new()  # Default weights
	var genome_b = WeightGenomeClass.create_random()  # Random weights

	print("Starting test battle: Default vs Random")
	var result := await run_single_battle(genome_a, genome_b)
	print("Battle completed!")
	print("  Winner: %s" % result.winner_side)
	print("  Duration: %.1fs" % result.duration)
	print("  Red damage dealt: %.0f" % result.red_damage_dealt)
	print("  Blue damage dealt: %.0f" % result.blue_damage_dealt)
