## Evolutionary Optimizer
## Uses genetic algorithms to evolve optimal GOAP agent weights.
##
## Algorithm:
## 1. Initialize population with random genomes
## 2. Run tournament - each genome fights every other
## 3. Select top performers (elitism)
## 4. Create next generation via crossover + mutation
## 5. Repeat until convergence or max generations
##
## Fitness is based on: wins, damage efficiency, survival time
class_name EvolutionaryOptimizer
extends Node

const WeightGenomeClass = preload("res://demo/ai/goap/optimization/weight_genome.gd")
const BattleSimulatorClass = preload("res://demo/ai/goap/optimization/battle_simulator.gd")

signal generation_completed(gen: int, best_fitness: float, avg_fitness: float)
signal optimization_completed(best_genome: Resource)
signal status_update(message: String)

## Configuration
@export_group("Population")
@export var population_size := 16  # Number of genomes per generation
@export var elite_count := 4  # Top performers kept unchanged
@export var max_generations := 50  # Stop after this many generations

@export_group("Mutation")
@export var mutation_rate := 0.15  # Chance of mutating each gene
@export var mutation_strength := 0.25  # How much mutations can change values

@export_group("Simulation")
@export var time_scale := 8.0  # Speed up battles
@export var max_battle_duration := 45.0  # Max seconds per battle
@export var battles_per_matchup := 2  # Fight each pair multiple times to reduce variance

@export_group("Convergence")
@export var convergence_threshold := 0.01  # Stop if fitness improvement < this
@export var convergence_generations := 5  # Check over this many generations

## State
var population: Array = []  # Array of WeightGenome resources
var current_generation := 0
var best_genome: Resource = null  # WeightGenome resource - now tracks current gen's best
var best_fitness := -INF  # Only used for display, not for selection
var fitness_history: Array[float] = []
var is_running := false

# Track the historical best for comparison purposes only
var _historical_best_fitness := -INF
var _historical_best_generation := 0

## Components
var _battle_simulator = null


func _ready() -> void:
	# Create battle simulator as child
	_battle_simulator = BattleSimulatorClass.new()
	_battle_simulator.time_scale = time_scale
	_battle_simulator.max_battle_duration = max_battle_duration
	add_child(_battle_simulator)


## Starts the evolutionary optimization process
func start_optimization() -> void:
	if is_running:
		push_warning("Optimization already running")
		return

	is_running = true
	current_generation = 0
	best_fitness = -INF
	_historical_best_fitness = -INF
	_historical_best_generation = 0
	fitness_history.clear()

	status_update.emit("Initializing population...")
	_initialize_population()

	status_update.emit("Starting evolution...")
	await _run_evolution()


## Initializes population with random genomes + one default
func _initialize_population() -> void:
	population.clear()

	# Add one genome with default (hand-tuned) weights
	var default_genome = WeightGenomeClass.new()
	default_genome.generation = 0
	population.append(default_genome)

	# Fill rest with random genomes
	for i in range(population_size - 1):
		var genome = WeightGenomeClass.create_random()
		genome.generation = 0
		population.append(genome)

	status_update.emit("Initialized population with %d genomes" % population_size)


## Main evolution loop
func _run_evolution() -> void:
	while is_running and current_generation < max_generations:
		status_update.emit("Generation %d/%d - Running tournament..." % [current_generation + 1, max_generations])

		# Reset fitness for all genomes
		for genome in population:
			genome.reset_fitness()

		# Run tournament
		await _run_tournament()

		# Calculate fitness scores
		for genome in population:
			genome.calculate_fitness()

		# Sort by fitness (descending)
		population.sort_custom(func(a, b): return a.fitness > b.fitness)

		# Always use current generation's winner as best genome
		# This is the correct approach because fitness is only meaningful within a generation
		# A genome that beats evolved opponents is better than one that dominated weak early opponents
		best_genome = population[0].duplicate_genome()
		best_fitness = population[0].fitness

		# Track historical best for informational purposes only
		if population[0].fitness > _historical_best_fitness:
			_historical_best_fitness = population[0].fitness
			_historical_best_generation = current_generation + 1
			status_update.emit("New historical high fitness: %.1f in gen %d (W:%d L:%d)" % [
				_historical_best_fitness, _historical_best_generation,
				population[0].wins, population[0].losses])

		status_update.emit("Gen %d winner: Fitness=%.1f (W:%d L:%d)" % [
			current_generation + 1, best_fitness, best_genome.wins, best_genome.losses])

		# Record fitness history
		var avg_fitness := _calculate_average_fitness()
		fitness_history.append(population[0].fitness)

		# Emit progress
		generation_completed.emit(current_generation, population[0].fitness, avg_fitness)

		# Print generation summary
		_print_generation_summary()

		# Check for convergence
		if _check_convergence():
			status_update.emit("Converged after %d generations" % (current_generation + 1))
			break

		# Create next generation
		status_update.emit("Creating generation %d..." % (current_generation + 2))
		_create_next_generation()
		current_generation += 1

	is_running = false
	status_update.emit("Optimization complete!")
	optimization_completed.emit(best_genome)


## Runs a round-robin tournament where each genome fights every other
func _run_tournament() -> void:
	var total_matchups := population_size * (population_size - 1) / 2
	var matchups_completed := 0

	for i in range(population_size):
		for j in range(i + 1, population_size):
			# Run multiple battles per matchup to reduce variance
			for _battle in range(battles_per_matchup):
				var result = await _battle_simulator.run_single_battle(
					population[i], population[j]
				)

				# Small delay between battles
				await get_tree().create_timer(0.1).timeout

			matchups_completed += 1

			# Progress update every 10 matchups
			if matchups_completed % 10 == 0:
				var pct := float(matchups_completed) / float(total_matchups) * 100.0
				status_update.emit("Tournament %.0f%% complete..." % pct)


## Creates the next generation via selection, crossover, and mutation
func _create_next_generation() -> void:
	var next_gen: Array = []

	# Elitism: Keep top performers unchanged
	for i in range(mini(elite_count, population.size())):
		var elite = population[i].duplicate_genome()
		elite.generation = current_generation + 1
		elite.reset_fitness()
		next_gen.append(elite)

	# Fill rest with offspring
	while next_gen.size() < population_size:
		# Tournament selection for parents
		var parent_a = _tournament_select()
		var parent_b = _tournament_select()

		# Ensure different parents
		var attempts := 0
		while parent_a == parent_b and attempts < 10:
			parent_b = _tournament_select()
			attempts += 1

		# Crossover
		var child = WeightGenomeClass.crossover(parent_a, parent_b)
		child.generation = current_generation + 1

		# Mutation
		child.mutate(mutation_rate, mutation_strength)

		next_gen.append(child)

	population = next_gen


## Tournament selection - pick best of k random individuals
func _tournament_select(k: int = 3) -> Resource:
	var candidates: Array = []

	for _i in range(k):
		var idx := randi() % population.size()
		candidates.append(population[idx])

	# Return the one with highest fitness
	candidates.sort_custom(func(a, b): return a.fitness > b.fitness)
	return candidates[0]


## Calculates average fitness across population
func _calculate_average_fitness() -> float:
	if population.is_empty():
		return 0.0

	var total := 0.0
	for genome in population:
		total += genome.fitness
	return total / population.size()


## Checks if optimization has converged
func _check_convergence() -> bool:
	if fitness_history.size() < convergence_generations:
		return false

	# Check if fitness improvement over last N generations is below threshold
	var recent := fitness_history.slice(-convergence_generations)
	var improvement: float = float(recent[-1]) - float(recent[0])
	var avg_improvement: float = improvement / float(convergence_generations)

	return abs(avg_improvement) < convergence_threshold


## Prints a summary of the current generation
func _print_generation_summary() -> void:
	print("\n=== Generation %d Summary ===" % (current_generation + 1))
	print("Gen %d Best: %.1f (this gen's winner will be used)" % [current_generation + 1, population[0].fitness])
	print("Historical high: %.1f (gen %d) - for reference only" % [_historical_best_fitness, _historical_best_generation])
	print("Avg fitness: %.1f" % _calculate_average_fitness())
	print("Top 3 genomes:")
	for i in range(mini(3, population.size())):
		var g = population[i]
		print("  %d. Fitness=%.1f W=%d L=%d DmgDealt=%.0f DmgTaken=%.0f" % [
			i + 1, g.fitness, g.wins, g.losses, g.damage_dealt, g.damage_taken
		])
	print("==============================\n")


## Saves the best genome to a file
func save_best_genome(path: String) -> Error:
	if not best_genome:
		push_error("No best genome to save")
		return ERR_DOES_NOT_EXIST

	var data: Dictionary = best_genome.to_dict()
	var json := JSON.stringify(data, "\t")

	var file := FileAccess.open(path, FileAccess.WRITE)
	if not file:
		return FileAccess.get_open_error()

	file.store_string(json)
	file.close()

	status_update.emit("Saved best genome to: " + path)
	return OK


## Loads a genome from file
func load_genome(path: String) -> Resource:
	var file := FileAccess.open(path, FileAccess.READ)
	if not file:
		push_error("Could not open file: " + path)
		return null

	var json := file.get_as_text()
	file.close()

	var data = JSON.parse_string(json)
	if not data is Dictionary:
		push_error("Invalid genome file format")
		return null

	return WeightGenomeClass.from_dict(data)


## Saves entire population to a file
func save_population(path: String) -> Error:
	var data := {
		"generation": current_generation,
		"best_fitness": best_fitness,
		"population": []
	}

	for genome in population:
		data["population"].append(genome.to_dict())

	var json := JSON.stringify(data, "\t")
	var file := FileAccess.open(path, FileAccess.WRITE)
	if not file:
		return FileAccess.get_open_error()

	file.store_string(json)
	file.close()
	return OK


## Loads population from file to resume optimization
func load_population(path: String) -> Error:
	var file := FileAccess.open(path, FileAccess.READ)
	if not file:
		return FileAccess.get_open_error()

	var json := file.get_as_text()
	file.close()

	var data = JSON.parse_string(json)
	if not data is Dictionary:
		return ERR_PARSE_ERROR

	current_generation = data.get("generation", 0)
	best_fitness = data.get("best_fitness", -INF)

	population.clear()
	for genome_data in data.get("population", []):
		var genome = WeightGenomeClass.from_dict(genome_data)
		population.append(genome)

	# Restore best genome
	if not population.is_empty():
		population.sort_custom(func(a, b): return a.fitness > b.fitness)
		best_genome = population[0].duplicate_genome()

	return OK


## Stops the optimization process
func stop_optimization() -> void:
	is_running = false
	status_update.emit("Optimization stopped by user")


## Returns current progress info
func get_progress() -> Dictionary:
	return {
		"generation": current_generation,
		"max_generations": max_generations,
		"best_fitness": best_fitness,  # Current gen's best
		"historical_best_fitness": _historical_best_fitness,
		"historical_best_generation": _historical_best_generation,
		"is_running": is_running,
		"population_size": population.size()
	}
