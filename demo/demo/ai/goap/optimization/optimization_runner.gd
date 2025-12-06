## Optimization Runner
## Main entry point for running GOAP weight optimization.
## Provides UI feedback and saves results.
##
## Usage:
## 1. Add this scene to your project
## 2. Run it to start optimization
## 3. Results are saved to user:// directory
##
## Or run headless:
##   godot --headless -s optimization_runner.gd
extends Node2D

const EvolutionaryOptimizerClass = preload("res://demo/ai/goap/optimization/evolutionary_optimizer.gd")
const WeightApplicatorClass = preload("res://demo/ai/goap/optimization/weight_applicator.gd")
const WeightGenomeClass = preload("res://demo/ai/goap/optimization/weight_genome.gd")

## Configuration
@export_group("Optimization Settings")
@export var population_size := 12
@export var max_generations := 30
@export var elite_count := 3
@export var mutation_rate := 0.15
@export var time_scale := 6.0
@export var battles_per_matchup := 2

@export_group("Output")
@export var output_dir := "user://goap_optimization/"
@export var auto_save_interval := 5  # Save every N generations

## UI References (optional)
var status_label: Label = null
var progress_bar: ProgressBar = null
var log_output: RichTextLabel = null

## Components
var optimizer: EvolutionaryOptimizerClass = null
var _generation_count := 0


func _ready() -> void:
	_setup_ui()
	_setup_optimizer()

	# Create output directory
	DirAccess.make_dir_recursive_absolute(output_dir.replace("user://", OS.get_user_data_dir() + "/"))

	# Auto-start in headless mode
	if DisplayServer.get_name() == "headless":
		_log("Running in headless mode")
		call_deferred("_start_optimization")
	else:
		_log("Press SPACE to start optimization")
		_log("Press S to stop")
		_log("Press L to load previous population")


func _setup_ui() -> void:
	# Create simple UI if running with display
	if DisplayServer.get_name() == "headless":
		return

	var canvas := CanvasLayer.new()
	add_child(canvas)

	var vbox := VBoxContainer.new()
	vbox.set_anchors_preset(Control.PRESET_FULL_RECT)
	vbox.add_theme_constant_override("separation", 10)
	canvas.add_child(vbox)

	var margin := MarginContainer.new()
	margin.add_theme_constant_override("margin_left", 20)
	margin.add_theme_constant_override("margin_right", 20)
	margin.add_theme_constant_override("margin_top", 20)
	margin.add_theme_constant_override("margin_bottom", 20)
	vbox.add_child(margin)

	var inner_vbox := VBoxContainer.new()
	inner_vbox.add_theme_constant_override("separation", 10)
	margin.add_child(inner_vbox)

	# Title
	var title := Label.new()
	title.text = "GOAP Weight Optimizer"
	title.add_theme_font_size_override("font_size", 32)
	inner_vbox.add_child(title)

	# Status
	status_label = Label.new()
	status_label.text = "Ready"
	status_label.add_theme_font_size_override("font_size", 18)
	inner_vbox.add_child(status_label)

	# Progress bar
	progress_bar = ProgressBar.new()
	progress_bar.custom_minimum_size = Vector2(400, 30)
	progress_bar.value = 0
	inner_vbox.add_child(progress_bar)

	# Log output
	log_output = RichTextLabel.new()
	log_output.custom_minimum_size = Vector2(600, 400)
	log_output.scroll_following = true
	log_output.bbcode_enabled = true
	inner_vbox.add_child(log_output)

	# Instructions
	var instructions := Label.new()
	instructions.text = "SPACE: Start | S: Stop | L: Load | R: Export Results"
	inner_vbox.add_child(instructions)


func _setup_optimizer() -> void:
	optimizer = EvolutionaryOptimizerClass.new()
	optimizer.population_size = population_size
	optimizer.max_generations = max_generations
	optimizer.elite_count = elite_count
	optimizer.mutation_rate = mutation_rate
	optimizer.time_scale = time_scale
	optimizer.battles_per_matchup = battles_per_matchup

	optimizer.status_update.connect(_on_status_update)
	optimizer.generation_completed.connect(_on_generation_completed)
	optimizer.optimization_completed.connect(_on_optimization_completed)

	add_child(optimizer)


func _input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed:
		match event.keycode:
			KEY_SPACE:
				_start_optimization()
			KEY_S:
				_stop_optimization()
			KEY_L:
				_load_population()
			KEY_R:
				_export_results()


func _start_optimization() -> void:
	if optimizer.is_running:
		_log("Optimization already running")
		return

	_log("[color=green]Starting optimization...[/color]")
	_log("Population: %d, Generations: %d" % [population_size, max_generations])
	_generation_count = 0

	optimizer.start_optimization()


func _stop_optimization() -> void:
	if not optimizer.is_running:
		_log("Optimization not running")
		return

	_log("[color=yellow]Stopping optimization...[/color]")
	optimizer.stop_optimization()


func _load_population() -> void:
	var path := output_dir + "population_latest.json"
	var err := optimizer.load_population(path)
	if err == OK:
		_log("[color=green]Loaded population from: %s[/color]" % path)
		_log("Resuming from generation %d" % optimizer.current_generation)
	else:
		_log("[color=red]Failed to load population[/color]")


func _export_results() -> void:
	if not optimizer.best_genome:
		_log("[color=red]No results to export[/color]")
		return

	# Export as GDScript
	var gdscript := WeightApplicatorClass.export_to_gdscript(optimizer.best_genome)
	var gdscript_path := output_dir + "optimized_weights.gd"

	var file := FileAccess.open(gdscript_path, FileAccess.WRITE)
	if file:
		file.store_string(gdscript)
		file.close()
		_log("[color=green]Exported GDScript to: %s[/color]" % gdscript_path)

	# Also save as JSON
	optimizer.save_best_genome(output_dir + "best_genome.json")


func _on_status_update(message: String) -> void:
	_log(message)
	if status_label:
		status_label.text = message


func _on_generation_completed(gen: int, best_fitness: float, avg_fitness: float) -> void:
	_generation_count = gen + 1

	if progress_bar:
		progress_bar.value = float(_generation_count) / float(max_generations) * 100.0

	_log("Gen %d: Best=%.1f, Avg=%.1f" % [_generation_count, best_fitness, avg_fitness])

	# Auto-save periodically
	if _generation_count % auto_save_interval == 0:
		_auto_save()


func _on_optimization_completed(best_genome: Resource) -> void:
	_log("[color=green]========================================[/color]")
	_log("[color=green]OPTIMIZATION COMPLETE![/color]")
	_log("[color=green]========================================[/color]")

	if best_genome:
		_log("Best genome stats:")
		_log("  Fitness: %.1f" % best_genome.fitness)
		_log("  Wins: %d, Losses: %d" % [best_genome.wins, best_genome.losses])
		_log("  Damage dealt: %.0f" % best_genome.damage_dealt)
		_log("  Damage taken: %.0f" % best_genome.damage_taken)

		# Auto-export
		_export_results()

		# Print key weights
		_log("")
		_log("Key optimized values:")
		_log("  Melee attack cost: %d (default: 8)" % best_genome.cost_melee_attack)
		_log("  Approach cost: %d (default: 4)" % best_genome.cost_approach_target)
		_log("  Cover vs melee cost: %d (default: 100)" % best_genome.cost_cover_vs_melee)
		_log("  Retreat vs melee cost: %d (default: 1)" % best_genome.cost_retreat_vs_melee)
		_log("  Goal switch cooldown: %.2fs (default: 0.5)" % best_genome.goal_switch_cooldown)
	else:
		_log("[color=red]No best genome found[/color]")


func _auto_save() -> void:
	# Save population
	var pop_path := output_dir + "population_latest.json"
	optimizer.save_population(pop_path)

	# Save best genome
	if optimizer.best_genome:
		var best_path := output_dir + "best_genome_gen%d.json" % _generation_count
		optimizer.save_best_genome(best_path)

	_log("[color=gray]Auto-saved at generation %d[/color]" % _generation_count)


func _log(message: String) -> void:
	var timestamp := Time.get_time_string_from_system()
	var full_msg := "[%s] %s" % [timestamp, message]

	print(full_msg.replace("[color=green]", "").replace("[color=red]", "").replace("[color=yellow]", "").replace("[color=gray]", "").replace("[/color]", ""))

	if log_output:
		log_output.append_text(full_msg + "\n")


## Quick test function for development
func _run_quick_test() -> void:
	_log("Running quick test with 4 genomes, 3 generations...")

	optimizer.population_size = 4
	optimizer.max_generations = 3
	optimizer.battles_per_matchup = 1

	optimizer.start_optimization()
