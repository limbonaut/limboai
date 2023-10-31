LimboAI - Behavior Trees and State Machines for Godot 4
---
LimboAI is a C++ module for Godot Engine 4 that provides an implementation of Behavior Trees and State Machines, which can be used together to create complex AI behaviors.

>**🛈 Supported Godot Engine: 4.2**

>**🛈 Previously supported:** [godot-4.1](https://github.com/limbonaut/limboai/tree/godot-4.1)

>**🛈 License**: Use of this source code is governed by an MIT-style license that can be found in the LICENSE file or at https://opensource.org/licenses/MIT.

A Behavior Tree (BT) is a powerful hierarchical structure used to model and control the behavior of agents in a game. It comprises tasks that represent specific actions or decision-making rules. When executed, the Behavior Tree starts from the root task and traverses down to the leaf tasks, which correspond to the actual actions or behaviors that the agent should perform. For detailed information on how various BT tasks function, please refer to the class documentation. The BehaviorTree class serves as a good starting point.

> 🛈 See also: [Introduction to Behavior Trees](./doc/getting-started.md).

![Textured screenshot](doc/images/behavior-tree-editor.png)

![Textured screenshot](doc/images/behavior-tree-debugger.png)

## Features

- **Behavior Trees (BT):**
    - Use the `BTPlayer` node to execute `BehaviorTree` resources.
    - Easily create, edit, and save `BehaviorTree` resources within the editor.
    - Combine and nest tasks in a hierarchy to create complex behaviors.
    - Control the flow of execution using composite, decorator, and condition tasks.
    - Write your own tasks by extending core classes: `BTAction`, `BTCondition`, `BTDecorator`, and `BTComposite`.
    - Built-in class documentation. Check out the `BehaviorTree` and `BTTask` class documentation to get started.
    - Utilize the `Blackboard` for seamless data sharing between tasks.
    - Use the `BTSubtree` task to execute a tree from a different resource file, promoting organization and reusability.
    - Blackboard scopes separate namespaces of variables from subtrees and enable advanced techniques like sharing data among agents in a group.
    - Visual Debugger: Inspect the execution of any BT in a running scene to identify and troubleshoot issues.
    - Evaluate the performance of your trees with custom performance monitors.

- **Hierarchical State Machines (HSM):**
    - Extend the `LimboState` class to implement state logic.
    - The `LimboHSM` node serves as a state machine that manages `LimboState` instances and transitions.
    - `LimboHSM` is a state itself and can be nested within other `LimboHSM` instances.
    - Event-based: Transitions are associated with events and are triggered by the state machine when the relevant event is dispatched, allowing for better decoupling of transitions from state logic.
    - Combine state machines with behavior trees using `BTState` for advanced reactive AI.
    - Delegation: Instead of extending `LimboState`, utilize the vanilla `LimboState` and delegate implementation to provided callback functions. Perfect for game jams and quick prototyping.
    - 🛈 Note: Currently, state machine transition setup and initialization must be done through code as there is no GUI editor for state machines.

- **Tested:** Behavior tree tasks and HSM have been covered by unit tests.

## Getting LimboAI

### Precompiled builds

- For the most recent builds, navigate to **Actions** → [**All Builds**](https://github.com/limbonaut/limboai/actions/workflows/all_builds.yml), select a build from the list, and scroll down until you find the **Artifacts** section.
- For release builds, check [**Releases**](https://github.com/limbonaut/limboai/releases).

### Compiling from source

- Download the Godot Engine source code and put this module source into the `modules/limboai` directory.
- Consult the Godot Engine documentation for instructions on [how to build from source code](https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html).
- If you plan to export a game utilizing the LimboAI module, you'll also need to build export templates.
- To execute unit tests, compile the engine with `tests=yes` and run it with `--test --tc="*[LimboAI]*"`.

## Using the module

<!-- - [Getting Started](./doc/getting-started.md#getting-started) -->
- [Introduction to Behavior Trees](./doc/getting-started.md#introduction-to-behavior-trees)
- [Creating custom tasks in GDScript](./doc/getting-started.md#creating-custom-tasks-in-gdscript)
    - [Task anatomy](./doc/getting-started.md#task-anatomy)
    - [Custom task example](./doc/getting-started.md#custom-task-example)

## Contributing

All contributions are welcome! Feel free to open issues with bug reports and feature requests, and submit PRs.

## Roadmap

Features and improvements that may be implemented in the future:
- ~~Providing precompiled builds for download.~~ 🗸
- ~~Tests and CI.~~ 🗸
- Creating a non-trivial demo project to showcase the capabilities of LimboAI.
- Exploring the execution history of Behavior Trees in the Visual Debugger.
- Expanding the library of tasks that can be optionally included in the build.
- Implementing an ignore list for tasks that users may want to hide in the task panel.
- GUI editor for state machines.
- Supporting GDExtension in the future, once it matures.
