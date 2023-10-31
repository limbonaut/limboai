# Getting Started

> **🛈 Getting the module:** [README: Getting LimboAI](../README.md#getting-limboai).


## TL;DR

- To create your own actions, extend the `BTAction` class.
- To create your own conditions, extend the `BTCondition` class.
- Use script template (available in "Misc → Create script template")


## Introduction to Behavior Trees

**Behavior Trees (BT)** are hierarchical structures used to model and control the behavior of agents in a game (e.g., characters, enemies, entities). They are designed to make it easier to create complex and highly modular behaviors for your games.

Behavior Trees are composed of tasks that represent specific actions or decision-making rules. Tasks can be broadly categorized into two main types: control tasks and leaf tasks. Control tasks determine the execution flow within the tree. They include `BTSequence`, `BTSelector`, and `BTInvert`. Leaf tasks represent specific actions to perform, like moving or attacking, or conditions that need to be checked. The `BTTask` class provides the foundation for various building blocks of the Behavior Trees. BT tasks can share data with the help of `Blackboard`.

> **🛈 Note:** To create your own actions, extend the `BTAction` class.

The BehaviorTree is executed from the root task and follows the rules specified by the control tasks, all the way down to the leaf tasks, which represent the actual actions that the agent should perform or conditions that should be checked. Each task returns a status when it is executed. It can be `SUCCESS`, `RUNNING`, or `FAILURE`. These statuses determine how the tree progresses. They are defined in `BT.Status`.

Behavior Trees handle conditional logic using condition tasks. These tasks check for specific conditions and return either `SUCCESS` or `FAILURE` based on the state of the agent or its environment (e.g., "IsLowOnHealth", "IsTargetInSight"). Conditions can be used together with `BTSequence` and `BTSelector` to craft your decision-making logic.

>**🛈 Note:** To create your own conditions, extend the `BTCondition` class.

Check out the `BTTask` class documentation in the editor, which provides the foundation for various building blocks of Behavior Trees.

## Creating custom tasks in GDScript

>**🛈 Note:** You can add a script template to your project with "Misc → Create script template" menu option.

### Task anatomy
```
@tool
extends BTAction

# Task parameters.
@export var parameter1: float
@export var parameter2: Vector2

## Note: Each method declaration is optional.
## At minimum, you only need to define the "_tick" method.

# Called to generate a display name for the task.
func _generate_name() -> String:
    return "MyTask"

# Called to initialize the task.
func _setup() -> void:
    pass

# Called when task is entered.
func _enter() -> void:
    pass

# Called when task is exited.
func _exit() -> void:
    pass

# Called each time this task is ticked (aka executed).
func _tick(delta: float) -> Status:
    return SUCCESS
```

### Custom task example

```gdscript
@tool
extends BTCondition

## InRange condition checks if the agent is within a range of target, defined by
## distance_min and distance_max.
## Returns SUCCESS if agent is within the defined range;
## otherwise, returns FAILURE.

@export var distance_min: float
@export var distance_max: float
@export var target_var := "target"

var _min_distance_squared: float
var _max_distance_squared: float


# Called to generate a display name for the task.
func _generate_name() -> String:
	return "InRange (%d, %d) of %s" % [distance_min, distance_max,
			LimboUtility.decorate_var(target_var)]


# Called to initialize the task.
func _setup() -> void:
	_min_distance_squared = distance_min * distance_min
	_max_distance_squared = distance_max * distance_max


# Called when task is executed.
func _tick(_delta: float) -> int:
	var target: Node2D = blackboard.get_var(target_var, null)
	if not is_instance_valid(target):
		return FAILURE

	var dist_sq: float = agent.global_position.distance_squared_to(target.global_position)
	if dist_sq >= _min_distance_squared and dist_sq <= _max_distance_squared:
		return SUCCESS
	else:
		return FAILURE
```
