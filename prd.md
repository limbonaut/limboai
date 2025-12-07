# PRD: GOAP Integration for LimboAI

## Project Overview

**Project Name:** LimboAI GOAP Extension
**Author:** Mark
**Date:** December 2, 2025
**Duration:** 7 days
**Repository:** Fork of https://github.com/limbonaut/limboai
**Target User:** Game designers using Godot Engine who want adaptive NPC behavior without scripting every decision path

### Executive Summary

This project extends LimboAI—a behavior tree and state machine plugin for Godot Engine—with Goal-Oriented Action Planning (GOAP). GOAP enables NPCs to dynamically plan sequences of actions to achieve goals, rather than following pre-scripted behavior trees. This creates more emergent, adaptive AI behavior similar to what made F.E.A.R. (2005) famous for its intelligent enemies.

---

## Problem Statement

### Current Limitations

LimboAI provides excellent tools for reactive AI:
- **Behavior Trees**: Define decision logic explicitly ("if X, do Y")
- **State Machines**: Manage discrete states with explicit transitions
- **Blackboard**: Share data between tasks

However, these systems require designers to **manually specify** every possible behavior path. For complex NPCs with many possible actions, this leads to:

1. **Combinatorial explosion**: BTs become unwieldy as action possibilities grow
2. **Brittle behavior**: NPCs can't adapt when expected paths fail
3. **Designer burden**: Every scenario must be anticipated and scripted

### The GOAP Solution

GOAP inverts the paradigm:
- Designers define **what actions exist** (preconditions + effects)
- Designers define **what goals to achieve** (desired world state)
- The AI **figures out the sequence** via planning

This enables:
- **Emergent behavior**: NPCs find solutions designers never anticipated
- **Adaptive replanning**: When actions fail, NPCs replan automatically
- **Easier scaling**: Adding new actions expands all NPCs' capabilities

---

## Goals & Objectives

### Primary Goals

1. **Implement a working GOAP planner** that integrates seamlessly with LimboAI's existing architecture
2. **Reuse LimboAI's Blackboard** as the world state representation
3. **Bridge GOAP plans to BT execution** so existing action implementations work
4. **Demonstrate with a compelling demo** showing emergent NPC behavior

### Success Criteria

| Criteria | Target |
|----------|--------|
| GOAP planner finds valid plans | A* search completes in <16ms for 10+ actions |
| Integration with Blackboard | Zero duplication of world state |
| Demo scenario | NPC dynamically plans 4+ action sequences |
| Code quality | Follows LimboAI patterns, includes tests |
| Documentation | Full README, architecture docs, setup guide |

---

## Technical Requirements

### New Components

#### 1. GOAPWorldState
Wraps LimboAI's Blackboard to provide GOAP-specific interface.

```cpp
class GOAPWorldState : public RefCounted {
    Ref<Blackboard> blackboard;

    bool satisfies(const Ref<GOAPWorldState> &goal) const;
    int distance_to(const Ref<GOAPWorldState> &goal) const;
    Ref<GOAPWorldState> duplicate() const;
};
```

**Requirements:**
- [ ] Read facts from existing Blackboard
- [ ] Support bool, int, float, and Object types
- [ ] Calculate heuristic distance between states (counting mismatches)
- [ ] Immutable operations (duplicate before modify)

**Heuristic Implementation:**

The `distance_to()` method uses a counting heuristic for A* admissibility:

```cpp
int GOAPWorldState::distance_to(const Ref<GOAPWorldState> &goal) const {
    int distance = 0;
    for (const auto &[fact_name, goal_value] : goal->facts) {
        Variant current_value = blackboard->get_var(fact_name, Variant());
        if (!values_match(current_value, goal_value)) {
            distance += 1;  // Each mismatched fact = 1 unit
        }
    }
    return distance;
}

bool GOAPWorldState::values_match(const Variant &current, const Variant &goal) const {
    if (current.get_type() != goal.get_type()) return false;

    switch (goal.get_type()) {
        case Variant::FLOAT:
            return Math::abs(float(current) - float(goal)) < 0.001f;
        default:
            return current == goal;
    }
}
```

**Why counting?** Each action changes at most N facts. Counting mismatches = minimum actions needed = admissible heuristic. Numeric distance doesn't map to action count.

#### 2. GOAPAction
Defines an action with preconditions, effects, and cost.

```cpp
class GOAPAction : public Resource {
    StringName action_name;
    Dictionary preconditions;  // fact_name → required_value
    Dictionary effects;        // fact_name → resulting_value
    int base_cost = 1;
    bool use_dynamic_cost = false;
    Ref<BehaviorTree> execution_tree;  // BT to run this action

    bool is_valid(const Ref<GOAPWorldState> &state) const;
    Ref<GOAPWorldState> apply_effects(const Ref<GOAPWorldState> &state) const;

    // Cost calculation
    int get_cost(Node *agent, Ref<Blackboard> bb) const;
    GDVIRTUAL3RC(int, _get_dynamic_cost, Node*, Ref<Blackboard>, int);  // Override for dynamic

    // Called at EXECUTION time only (not during planning)
    virtual bool check_procedural_preconditions(Node* agent, Ref<Blackboard> bb) const;
    GDVIRTUAL2RC(bool, _check_procedural_preconditions, Node*, Ref<Blackboard>);
};
```

**Requirements:**
- [ ] Saveable as Godot Resource (.tres)
- [ ] Editable in Godot Inspector
- [ ] Link to BehaviorTree for execution
- [ ] Support procedural precondition checks via virtual method (execution-time only)
- [ ] Default procedural check returns `true`
- [ ] Support dynamic cost calculation via virtual method

**Two-Phase Precondition Validation:**

| Phase | Method | When Called | Purpose |
|-------|--------|-------------|---------|
| Planning | `is_valid()` | During A* search | Fast Dictionary lookup only |
| Execution | `check_procedural_preconditions()` | Before starting action BT | Expensive checks (raycasts, pathfinding) |

This keeps planning fast while allowing runtime validation.

#### 3. GOAPGoal
Defines a target world state to achieve.

```cpp
class GOAPGoal : public Resource {
    StringName goal_name;
    Dictionary target_state;  // fact_name → desired_value
    int priority;

    bool is_satisfied(const Ref<GOAPWorldState> &state) const;
};
```

**Requirements:**
- [ ] Partial state matching (only specified facts matter)
- [ ] Priority for goal selection
- [ ] Saveable as Resource

#### 4. GOAPPlanner
A* search algorithm to find optimal action sequences.

```cpp
class GOAPPlanner : public RefCounted {
    int max_iterations = 1000;

    // Stateless - actions passed per-call for multi-NPC support
    TypedArray<GOAPAction> plan(
        const Vector<Ref<GOAPAction>> &available_actions,
        const Ref<GOAPWorldState> &current,
        const Ref<GOAPGoal> &goal,
        Node *agent = nullptr,           // For dynamic costs
        Ref<Blackboard> bb = nullptr     // For dynamic costs
    );

    // Planning stats for debugging
    int get_last_iterations() const;
    float get_last_plan_time_ms() const;
};
```

**Requirements:**
- [ ] Backward-chaining A* search
- [ ] Configurable iteration limit (prevent infinite loops)
- [ ] Return empty array if no plan found
- [ ] Expose planning stats (iterations, time) for debugging
- [ ] Support dynamic cost evaluation when agent/blackboard provided

#### 5. BTRunGOAPPlan (BT Integration)
A BTTask that executes GOAP plans within behavior trees.

```cpp
class BTRunGOAPPlan : public BTAction {
    Ref<GOAPPlanner> planner;
    Ref<GOAPGoal> goal;
    Vector<Ref<GOAPAction>> available_actions;
    Ref<BehaviorTree> fallback_tree;  // Executed when no plan found
    float replan_cooldown = 0.2f;     // Minimum seconds between replans

    // Runtime state
    TypedArray<GOAPAction> current_plan;
    int current_action_index;
    Ref<BTInstance> current_action_instance;
    bool interrupt_requested = false;

    // Relevant facts tracking for smart replanning
    HashSet<StringName> relevant_facts;
    Dictionary cached_fact_values;

    Status _tick(double delta) override;
    void interrupt();  // Force replan on next tick

private:
    static thread_local int execution_depth;  // Recursion guard
    static const int MAX_DEPTH = 3;
};
```

**Requirements:**
- [ ] Generate plan on first tick (or when invalidated)
- [ ] Execute each action's BT sequentially
- [ ] Check procedural preconditions before starting each action
- [ ] Replan on hard failure (BT returns `FAILURE`)
- [ ] Replan on procedural precondition failure
- [ ] Replan on `interrupt()` call from external code
- [ ] Execute `fallback_tree` when no plan found
- [ ] Respect `replan_cooldown` to prevent oscillation
- [ ] Track relevant facts for smart replan detection
- [ ] Guard against fallback tree recursion (max depth 3)
- [ ] Expose plan to debugger

**Execution Flow:**

```cpp
Status BTRunGOAPPlan::_tick(double delta) {
    // Recursion guard
    if (execution_depth >= MAX_DEPTH) {
        ERR_PRINT("BTRunGOAPPlan: Max nesting depth exceeded");
        return FAILURE;
    }
    execution_depth++;

    // Phase 1: Need a plan?
    if (current_plan.is_empty() || should_replan()) {
        if (!respect_cooldown()) {
            current_plan = planner->plan(actions, world_state, goal, agent, bb);
            cache_relevant_facts();
        }
        if (current_plan.is_empty()) {
            execution_depth--;
            return execute_fallback();
        }
    }

    // Phase 2: Starting new action? Check procedural preconditions
    if (starting_new_action) {
        Ref<GOAPAction> action = current_plan[current_action_index];
        if (!action->check_procedural_preconditions(get_agent(), blackboard)) {
            invalidate_plan();  // Will replan next tick
            execution_depth--;
            return RUNNING;
        }
        current_action_instance = action->execution_tree->instantiate();
    }

    // Phase 3: Tick current action
    Status action_status = current_action_instance->tick(delta);

    if (action_status == SUCCESS) {
        current_action_index++;
        if (current_action_index >= current_plan.size()) {
            execution_depth--;
            return SUCCESS;  // Plan complete!
        }
    } else if (action_status == FAILURE) {
        invalidate_plan();  // Hard failure, will replan
    }

    execution_depth--;
    return RUNNING;
}
```

**Relevant Facts Tracking:**

Only replan when facts the current plan depends on change:

```cpp
void BTRunGOAPPlan::cache_relevant_facts() {
    relevant_facts.clear();
    cached_fact_values.clear();

    for (Ref<GOAPAction> action : current_plan) {
        for (const auto &[fact, value] : action->preconditions) {
            relevant_facts.insert(fact);
            cached_fact_values[fact] = blackboard->get_var(fact);
        }
    }
}

bool BTRunGOAPPlan::should_replan() {
    if (interrupt_requested) return true;

    for (const StringName &fact : relevant_facts) {
        if (blackboard->get_var(fact) != cached_fact_values[fact]) {
            return true;
        }
    }
    return false;
}
```

### Integration Points

#### Blackboard Integration
- GOAPWorldState reads directly from agent's Blackboard
- Actions' effects write back to Blackboard
- No separate world state storage

#### BehaviorTree Integration
- BTRunGOAPPlan is a standard BTTask
- Can be used anywhere in a behavior tree
- Each GOAPAction references a BT subtree for execution

#### Debugger Integration (Stretch)
- Show current plan in debug panel
- Visualize planning process
- Display why actions were rejected

---

## Architecture

### Directory Structure

```
limboai/
├── goap/
│   ├── goap_action.h
│   ├── goap_action.cpp
│   ├── goap_goal.h
│   ├── goap_goal.cpp
│   ├── goap_planner.h
│   ├── goap_planner.cpp
│   ├── goap_world_state.h
│   └── goap_world_state.cpp
│
├── bt/tasks/goap/
│   ├── bt_run_goap_plan.h
│   └── bt_run_goap_plan.cpp
│
└── editor/goap/              # Stretch goal
    ├── goap_debugger.h
    └── goap_debugger.cpp
```

### Class Hierarchy

```
RefCounted
├── GOAPWorldState
├── GOAPPlanner
└── Resource
    ├── GOAPAction
    └── GOAPGoal

BTTask
└── BTAction
    └── BTRunGOAPPlan
```

### Data Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Blackboard │────▶│ WorldState  │────▶│   Planner   │
│  (runtime)  │     │  (snapshot) │     │    (A*)     │
└─────────────┘     └─────────────┘     └──────┬──────┘
                                               │
                                               ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  BTPlayer   │◀────│BTRunGOAPPlan│◀────│ Plan[Action]│
│  (executes) │     │  (manages)  │     │  (result)   │
└─────────────┘     └─────────────┘     └─────────────┘
```

---

## Scope

### Tiered MVP Approach

To manage risk, the MVP is split into tiers with clear decision points:

#### Tier 1: Must Ship (Days 1-4)

| Component | Description |
|-----------|-------------|
| GOAPWorldState | Blackboard wrapper with counting heuristic |
| GOAPAction | Action resource with preconditions/effects (static cost only) |
| GOAPGoal | Goal resource with target state |
| GOAPPlanner | A* backward search implementation |
| BTRunGOAPPlan | Basic plan execution with replan-on-failure |
| Registration | Register classes with Godot |
| Unit Tests | Core planner logic |

**Tier 1 explicitly excludes:** Effect verification, procedural preconditions, fallback tree, dynamic costs

#### Tier 2: Should Ship (Day 5)

| Component | Description |
|-----------|-------------|
| Fallback Tree | Execute backup BT when no plan found |
| Replan Cooldown | Prevent oscillation |
| Procedural Preconditions | Runtime checks before action execution |
| Recursion Guard | Prevent fallback tree infinite loops |

#### Tier 3: Nice to Have (Days 6-7)

| Component | Description |
|-----------|-------------|
| Relevant Facts Tracking | Smart replan detection |
| Dynamic Costs | Virtual `_get_dynamic_cost()` method |
| Demo Scene | Combat NPC with full action set |
| Documentation | README, architecture docs |

#### Stretch Goals (Post-Challenge)

- Debugger panel to visualize current plan
- Plan caching with Blackboard versioning
- Visual action editor in Godot
- Multiple simultaneous goals with priority selection
- Parallel action execution

### Decision Points

| Checkpoint | Condition | Action |
|------------|-----------|--------|
| End of Day 2 | GOAPPlanner A* not working | Cut procedural preconditions entirely |
| End of Day 4 | BTRunGOAPPlan has bugs | Ship Tier 1 only, document limitations |
| Day 5 | Demo not working | Simplify to 3 actions, prove concept only |

### What to Cut First (if behind)

1. ~~Effect verification~~ (removed from MVP - too complex, causes thrashing)
2. Dynamic costs (static costs work fine for demo)
3. Relevant facts tracking (replanning is cheap for 10 actions)

### What to Protect

1. Core planner (the whole point of the project)
2. Fallback tree (essential for robust NPCs)
3. Replan cooldown (prevents obvious bugs)

---

## Test Criteria

### Planner Tests

| Test Case | Input | Expected Output |
|-----------|-------|-----------------|
| **Valid Plan** | Goal reachable with 3 actions | Returns ordered action sequence |
| **Impossible Goal** | Goal requires fact no action produces | Returns empty array |
| **Circular Dependencies** | Action A needs B's effect, B needs A's | Returns empty array (no infinite loop) |
| **Cost Optimization** | Two valid paths with different costs | Returns lower total cost path |
| **Already Satisfied** | Goal already true in world state | Returns empty array (no actions needed) |
| **Empty Action Set** | No actions provided | Returns empty array gracefully |
| **Iteration Limit** | Complex problem exceeds `max_iterations` | Returns empty array, doesn't hang |

### Precondition Tests

| Test Case | Input | Expected Output |
|-----------|-------|-----------------|
| **Unmet Preconditions** | Action preconditions not satisfied | Action excluded from plan |
| **Partial Match** | 2 of 3 preconditions met | Action excluded |
| **Procedural Check Fails** | `check_procedural_preconditions()` returns false | Action skipped, replan triggered |
| **Procedural Check Passes** | Virtual method returns true | Action executes normally |

### Effect Tests

| Test Case | Input | Expected Output |
|-----------|-------|-----------------|
| **Effect Application** | Apply action effects to world state | State correctly modified |
| **Immutability** | Apply effects | Original state unchanged, new state returned |
| **Partial Goal Match** | Goal has 2 facts, world has 5 | Only specified facts compared |

### Integration Tests

| Test Case | Input | Expected Output |
|-----------|-------|-----------------|
| **Replanning on Failure** | Action BT returns `FAILURE` | New plan generated |
| **Procedural Replan** | Procedural precondition fails | New plan generated |
| **Interrupt Handling** | `interrupt()` called mid-plan | Replan on next tick |
| **Fallback Execution** | No plan found, fallback tree set | Fallback tree executes |
| **Cooldown Respected** | Rapid replan requests | Only one replan per cooldown period |
| **Recursion Guard** | Fallback contains BTRunGOAPPlan | Max depth enforced, no infinite loop |
| **Relevant Facts** | Unrelated Blackboard var changes | No replan triggered |

---

## Demo Scenario

### "Tactical Combat NPC"

An enemy NPC that must eliminate the player using available resources.

#### World State Facts
```
has_weapon: bool
weapon_loaded: bool
in_cover: bool
target_visible: bool
target_in_range: bool
target_dead: bool
health_low: bool
near_weapon_pickup: bool
near_cover: bool
near_ammo: bool
target_in_cover: bool
```

#### Available Actions

| Action | Preconditions | Effects | Cost |
|--------|---------------|---------|------|
| GoToWeapon | — | near_weapon_pickup=true | 3 |
| PickUpWeapon | near_weapon_pickup | has_weapon=true, near_weapon_pickup=false | 1 |
| GoToAmmo | has_weapon | near_ammo=true | 3 |
| LoadWeapon | has_weapon, near_ammo | weapon_loaded=true | 1 |
| GoToCover | — | near_cover=true | 2 |
| TakeCover | near_cover | in_cover=true | 1 |
| LeaveCover | in_cover | in_cover=false, near_cover=false | 1 |
| ApproachTarget | has_weapon | target_in_range=true, target_visible=true | 4 |
| Flank | target_in_cover, has_weapon | target_visible=true, target_in_cover=false | 5 |
| Shoot | has_weapon, weapon_loaded, target_in_range, target_visible | target_dead=true | 1 |
| MeleeAttack | target_in_range, target_visible | target_dead=true | 8 |
| Retreat | health_low | in_cover=true, near_cover=true, target_in_range=false | 3 |

**Design Notes:**
- `Retreat` produces effects so the planner will select it when health is low
- `MeleeAttack` has high cost (8) to discourage over ranged combat
- `Flank` handles the case where target is behind cover
- `ApproachTarget` grants `target_visible` (line of sight from proximity)
- `PickUpWeapon` clears `near_weapon_pickup` to prevent repeated pickups

#### Goals

**Primary Goal (default):**
```
target_dead: true
```

**Survival Goal (activated when health_low=true):**
```
in_cover: true
```

Goal selection logic (in BTRunGOAPPlan or parent BT):
```
if health_low and not in_cover:
    active_goal = survival_goal
else:
    active_goal = kill_goal
```

#### Demo Flow

1. **Scene starts**: NPC has no weapon, player is across the room
2. **Initial plan**: `GoToWeapon → PickUpWeapon → GoToAmmo → LoadWeapon → ApproachTarget → Shoot`
3. **Player ducks behind cover**: `target_visible=false`, `target_in_cover=true`
4. **Replan**: `Flank → Shoot` (NPC moves to flank position)
5. **NPC takes damage**: `health_low=true`, survival goal activates
6. **Replan with new goal**: `Retreat` (gets to cover, clears target_in_range)
7. **Health restored** (via pickup or time): primary goal reactivates
8. **Final plan**: `LeaveCover → ApproachTarget → Shoot`

This demonstrates:
- Emergent multi-step planning
- Adaptive replanning when world changes
- Goal switching based on NPC state
- Flanking behavior without explicit scripting

---

## Technical Decisions

### Why Backward Search?
Forward search from current state has unbounded branching. Backward search from goal constrains the search space to only relevant actions.

### Why Wrap Blackboard (Not Replace)?
- Existing BT tasks already use Blackboard
- Scoping and sharing already implemented
- Single source of truth for world state

### Why BT Subtrees for Action Execution?
- Reuses existing action implementations
- Complex actions (navigation, animation) already work
- Designers familiar with BT authoring

### Why Resource-Based Actions?
- Saveable/loadable like BehaviorTree resources
- Editable in Inspector
- Shareable across NPCs
- Version controllable

### Why Counting Heuristic?
- A* requires admissible heuristic (never overestimates)
- Each action changes N facts → counting mismatches = minimum actions
- Numeric distance doesn't map to action count
- Simple to implement, guaranteed correct

### Why Two-Phase Preconditions?
- Planning must be fast (called in A* loop)
- Expensive checks (raycasts, pathfinding) would cause frame spikes
- Check procedurally right before execution instead
- If check fails, replan with new world knowledge

### Build Target
- **Primary:** GDExtension (update `SConstruct` to include `goap/` sources)
- **Secondary:** Engine Module (update `SCsub` to include `goap/` sources)
- Both builds follow existing LimboAI patterns with minimal changes

### Blackboard Scoping
- `GOAPWorldState` inherits LimboAI's existing Blackboard scoping behavior
- Reading facts uses `get_var()` which automatically walks parent scopes
- Writing effects targets the agent's local Blackboard by default
- Shared state changes (e.g., `alarm_raised`) should be handled explicitly by action BT subtrees

### Multi-NPC Architecture
- `GOAPPlanner` is **stateless** and can be shared across multiple NPCs
- Available actions are passed per-call to `plan()`, not stored internally
- Runtime state (current plan, action index) lives in `BTRunGOAPPlan` task instance
- Different NPC types can have different action sets

---

## Replanning Strategy

### When to Replan

Use **event-driven replanning** rather than continuous planning:

| Trigger | Description |
|---------|-------------|
| **Action Failure** | BT subtree returns `FAILURE` |
| **Procedural Check Failure** | `check_procedural_preconditions()` returns false |
| **Relevant Fact Changed** | A fact the plan depends on changed in Blackboard |
| **External Interrupt** | `interrupt()` called (damage, new threat, goal change) |

### Failure Types

| Type | Condition | Response |
|------|-----------|----------|
| **Hard Failure** | BT subtree returns `FAILURE` | Immediate replan |
| **Procedural Failure** | Precondition check fails before action | Immediate replan |
| **World Changed** | Relevant fact differs from cached value | Replan before next action |
| **Interrupt** | External code calls `interrupt()` | Replan on next tick |

### Oscillation Prevention

To prevent rapid plan switching:
- **Plan Commitment:** Once an action starts, it must complete or explicitly fail
- **Cooldown:** Minimum 200ms (configurable) between replans
- **Relevant Facts Only:** Don't replan for unrelated Blackboard changes

### Fallback Behavior

When no valid plan is found:
1. Execute `fallback_tree` if configured
2. If no fallback: return `FAILURE` (let parent BT handle)
3. Retry planning on next `interrupt()` or relevant world change

**Recommended fallback tree:**
```
Selector
├── Sequence [if near_cover]
│   └── TakeCover
├── Sequence [if health_low]
│   └── MoveTowardSpawn
└── IdleAlert
```

**Recursion Protection:** Fallback trees containing `BTRunGOAPPlan` are allowed but limited to depth 3. Exceeding depth returns `FAILURE` with error log.

---

## Risks & Mitigations

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| A* performance issues | Medium | High | Iteration limits, counting heuristic, plan caching |
| GDExtension build issues | Medium | Medium | Keep module build as fallback |
| Scope creep | High | Medium | Tiered MVP with decision points |
| Unfamiliar with C++ | High | High | AI assistance, follow LimboAI patterns strictly |
| Integration breaks existing features | Low | High | Unit tests, don't modify core files |
| Procedural checks slow planning | Medium | Medium | Two-phase validation (plan vs execute) |
| Replan oscillation | Medium | Medium | Cooldown + relevant facts tracking |

---

## Timeline

### Day 1-2: Core Classes
- [ ] Build environment working
- [ ] GOAPWorldState implemented with counting heuristic
- [ ] GOAPAction implemented (static cost)
- [ ] GOAPGoal implemented
- [ ] Basic unit tests for world state

### Day 3-4: Planner & Integration
- [ ] GOAPPlanner A* implementation
- [ ] BTRunGOAPPlan task (Tier 1: basic replan-on-failure)
- [ ] Class registration with Godot
- [ ] Planner unit tests
- [ ] **Decision point:** Assess progress, cut scope if needed

### Day 5: Tier 2 Features
- [ ] Fallback tree support
- [ ] Procedural preconditions (execution-time)
- [ ] Replan cooldown
- [ ] Recursion guard
- [ ] Integration tests

### Day 6: Demo Scene
- [ ] Combat NPC setup
- [ ] Action BT subtrees (movement, shooting, etc.)
- [ ] World state configuration
- [ ] Demo working end-to-end
- [ ] Relevant facts tracking (if time)

### Day 7: Polish & Documentation
- [ ] Edge case handling
- [ ] Performance testing
- [ ] README complete
- [ ] Architecture docs
- [ ] Demo video recorded

---

## Deliverables

1. **Forked Repository** with clear commit history
2. **Working Demo** showing tactical NPC behavior
3. **Documentation**
   - README with setup instructions
   - Architecture overview
   - API reference for new classes
4. **Demo Video** (5 minutes)
   - Feature walkthrough
   - Technical explanation
   - AI-assisted development reflection

---

## API Surface for GDScript

All new classes are accessible from GDScript:

```gdscript
# Creating actions
var shoot_action = GOAPAction.new()
shoot_action.action_name = "Shoot"
shoot_action.preconditions = {"has_weapon": true, "target_visible": true}
shoot_action.effects = {"target_dead": true}
shoot_action.base_cost = 1
shoot_action.execution_tree = preload("res://ai/actions/shoot.tres")

# Creating goals
var kill_goal = GOAPGoal.new()
kill_goal.goal_name = "KillTarget"
kill_goal.target_state = {"target_dead": true}
kill_goal.priority = 10

# Manual planning (for debugging)
var planner = GOAPPlanner.new()
var world_state = GOAPWorldState.new()
world_state.blackboard = agent.get_blackboard()

var plan = planner.plan(actions, world_state, kill_goal)
print("Plan: ", plan.map(func(a): return a.action_name))
print("Took ", planner.get_last_plan_time_ms(), "ms")

# Subclassing for dynamic costs
class_name GoToWeaponAction extends GOAPAction

func _get_dynamic_cost(agent: Node, bb: Blackboard, base: int) -> int:
    var weapon_pos = bb.get_var("nearest_weapon_position")
    var distance = agent.global_position.distance_to(weapon_pos)
    return base + int(distance / 100.0)

# Subclassing for procedural preconditions
class_name ShootAction extends GOAPAction

func _check_procedural_preconditions(agent: Node, bb: Blackboard) -> bool:
    var target = bb.get_var("target")
    if not target:
        return false
    # Raycast to check line of sight
    var space = agent.get_world_3d().direct_space_state
    var query = PhysicsRayQueryParameters3D.create(agent.global_position, target.global_position)
    var result = space.intersect_ray(query)
    return result.is_empty() or result.collider == target
```

---

## References

- [Original GOAP Paper - Jeff Orkin](http://alumni.media.mit.edu/~jorkin/goap.html)
- [F.E.A.R. AI GDC Talk](https://www.gdcvault.com/play/1022019/Three-States-and-a-Plan)
- [Building the AI of F.E.A.R. - Game Developer](https://www.gamedeveloper.com/design/building-the-ai-of-f-e-a-r-with-goal-oriented-action-planning)
- [LimboAI Documentation](https://limboai.readthedocs.io/)
- [Godot GDExtension Guide](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/index.html)
