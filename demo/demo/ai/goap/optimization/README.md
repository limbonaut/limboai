# GOAP Weight Optimization System

This system uses evolutionary algorithms to automatically discover optimal weights for GOAP agents, making them more competitive through self-play.

## Overview

The optimizer evolves populations of weight configurations through:
1. **Tournament Selection** - Agents fight each other; winners advance
2. **Crossover** - Combine successful traits from parent configurations
3. **Mutation** - Random variations to explore new strategies
4. **Elitism** - Top performers survive unchanged

## Quick Start

### Run the Optimizer

1. Open the optimization scene:
   ```
   res://demo/ai/goap/optimization/optimization_runner.tscn
   ```

2. Run the scene and press **SPACE** to start optimization

3. Results are saved to `user://goap_optimization/`

### Headless Mode (Faster)

Run from command line for faster iteration:
```bash
godot --headless --path /path/to/project res://demo/ai/goap/optimization/optimization_runner.tscn
```

## Files

| File | Description |
|------|-------------|
| `weight_genome.gd` | Defines all tunable weights as a Resource |
| `battle_simulator.gd` | Runs battles and tracks results |
| `evolutionary_optimizer.gd` | Main evolutionary algorithm |
| `weight_applicator.gd` | Applies optimized weights to agents |
| `genome_aware_action.gd` | Base class for genome-aware actions |
| `optimization_runner.gd/.tscn` | Main entry point with UI |

## Weight Categories

### Action Base Costs
Control how expensive each action is for the GOAP planner:
- `cost_go_to_weapon`, `cost_pickup_weapon`
- `cost_go_to_ammo`, `cost_load_weapon`
- `cost_go_to_cover`, `cost_take_cover`, `cost_leave_cover`
- `cost_approach_target`, `cost_flank`
- `cost_shoot`, `cost_melee_attack`, `cost_retreat`

### Dynamic Cost Modifiers
Context-sensitive adjustments:
- `mod_preferred_weapon_bonus` - Bonus for preferred weapon (-3 default)
- `mod_nonpreferred_weapon_penalty` - Penalty for non-preferred (+5 default)
- `mod_close_pickup_bonus` - Bonus when pickup is close
- `mod_counter_weapon_bonus` - Bonus when countering enemy weapon type

### Threat-Based Costs
Override costs based on threat type:
- `cost_cover_vs_melee` - Cover cost against melee (100 = avoid)
- `cost_cover_vs_ranged` - Cover cost against ranged (1 = prefer)
- `cost_retreat_vs_melee` - Retreat cost against melee (1 = prefer)

### Goal Timing (Hysteresis)
Controls goal switching behavior:
- `goal_switch_cooldown` - Min time between switches (0.5s)
- `defensive_goal_commitment` - Min time in defensive goals (1.0s)
- `attack_goal_commitment` - Min time in attack goal (0.3s)
- `defensive_timeout` - Max defensive time before attack (4.0s)

### Combat Thresholds
Distance and health triggers:
- `retreat_distance` - Distance ranged wants to maintain (300)
- `close_gap_threshold` - Distance melee wants to close to (150)
- `too_close_threshold` - Distance ranged feels threatened (200)
- `low_health_threshold` - Health to trigger survival mode (50)

## Integrating Optimized Weights

### Option 1: Apply at Runtime

```gdscript
var genome = WeightGenome.load("user://goap_optimization/best_genome.json")
WeightApplicator.apply_to_agent(agent, genome)
```

### Option 2: Export to Code

The optimizer generates GDScript constants you can paste into `goap_config.gd`:

```gdscript
# From user://goap_optimization/optimized_weights.gd
const COST_MELEE_ATTACK := 6  # Evolved from default 8
const COST_RETREAT := 2       # Evolved from default 3
```

### Option 3: Make Actions Genome-Aware

Update actions to read from genome:

```gdscript
# Before
return 100  # hardcoded

# After
return WeightApplicator.get_weight(agent, "cost_cover_vs_melee", 100)
```

## Configuration

Edit `optimization_runner.gd` exports or modify at runtime:

```gdscript
@export var population_size := 12      # More = better coverage, slower
@export var max_generations := 30      # More = better convergence
@export var elite_count := 3           # Top N kept unchanged
@export var mutation_rate := 0.15      # Higher = more exploration
@export var time_scale := 6.0          # Battle speed multiplier
@export var battles_per_matchup := 2   # Reduce variance
```

## Fitness Function

Agents are scored on:
1. **Wins** (100 points each) - Most important
2. **Damage Efficiency** (dealt - taken) * 0.5
3. **Survival Time** * 0.1 - Tiebreaker

## Tips for Better Results

1. **Run overnight** - More generations = better results
2. **Increase population** - 20-30 for production runs
3. **Multiple battles per matchup** - Reduces randomness
4. **Save checkpoints** - Resume from `population_latest.json`
5. **Test against hand-tuned** - Include default genome in initial population

## Example Output

After optimization, you might see evolved weights like:

```
Generation 30 Summary
Best fitness: 850.0
Avg fitness: 420.5
Top genome: W=8 L=2 DmgDealt=1200 DmgTaken=400

Key changes from defaults:
- cost_melee_attack: 8 -> 5 (more aggressive melee)
- cost_retreat: 3 -> 1 (quicker to retreat)
- defensive_timeout: 4.0 -> 2.5 (shorter defensive phases)
```
