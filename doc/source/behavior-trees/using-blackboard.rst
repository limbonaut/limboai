.. _blackboard:

Sharing data using Blackboard
=============================

To share data between different tasks and states, we employ a feature known as the :ref:`Blackboard<class_Blackboard>`.
The :ref:`Blackboard<class_Blackboard>` serves as a central repository where tasks and states can store and retrieve named variables,
allowing for seamless data interchange. Each instance of a behavior tree or a state machine gets its own dedicated :ref:`Blackboard<class_Blackboard>`. It has the capability to store various data types,
including objects and resources.

Using the :ref:`Blackboard<class_Blackboard>`, you can easily share data in your behavior trees, making the tasks in the behavior tree more flexible.

.. _accessing_blackboard:

Accessing the Blackboard in a Task
----------------------------------

Every :ref:`BTTask<class_BTTask>` has access to the :ref:`Blackboard<class_Blackboard>`, providing a
straightforward mechanism for data exchange.
Here's an example of how you can interact with the :ref:`Blackboard<class_Blackboard>` in GDScript:

.. code:: gdscript

    @export var speed_var: StringName = &"speed"

    func _tick(delta: float) -> Status:
        # Set the value of the "speed" variable:
        blackboard.set_var(speed_var, 200.0)

        # Get the value of the "speed" variable, with a default value of 100.0 if not found:
        var speed: float = blackboard.get_var(speed_var, 100.0)

        # Check if the "speed" variable exists:
        if blackboard.has_var(speed_var):
            # ...

If you are accessing a variable that holds an object instance, and it is
expected that the instance may be null or freed, you can do it like this:

.. code:: gdscript

    @export var object_var: StringName = &"object"

    func _tick(delta: float) -> Status:
        # Get object instance stored in the "object" variable.
        # - Important: Avoid specifying a type for "obj" in GDScript
        #   to prevent errors when the instance is freed.
        var obj = blackboard.get_var(object_var)
        if is_instance_valid(obj):
            # ...

It is recommended to suffix variable name properties with ``_var``, like in the example above, which enables the
inspector to provide a more convenient property editor for the variable. This editor
allows you to select or add the variable to the blackboard plan, and provides a
warning icon if the variable does not exist in the blackboard plan.

    **🛈 Note:** The variable doesn't need to exist when you set it in code.

.. _editing_plan:

Editing the Blackboard Plan
---------------------------

The Blackboard Plan, associated with each :ref:`BehaviorTree<class_BehaviorTree>`
resource, dictates how the :ref:`Blackboard<class_Blackboard>` initializes for each
new instance of the :ref:`BehaviorTree<class_BehaviorTree>`.
BlackboardPlan resource stores default values, type information, and data bindings
necessary for :ref:`BehaviorTree<class_BehaviorTree>` initialization.

To add, modify, or remove variables from the Blackboard Plan, follow these steps:

1. Open the LimboAI editor and load the behavior tree you want to edit.
2. In the editor, click on the small button located inside the tab. This will open the :ref:`BlackboardPlan<class_BlackboardPlan>` in the Inspector.
3. In the Inspector, click the "Manage..." button to show the blackboard plan editor.
4. In the blackboard plan editor, you can add, remove, or reorder variables, and modify their data type and hint.
5. The hint provides additional information about the variable to the Inspector, such as minimum and maximum values for an integer variable. Learn more about `property hints in the official Godot documentation <https://docs.godotengine.org/en/stable/classes/class_%40globalscope.html#enum-globalscope-propertyhint>`_.
6. You can specify the default values of the variables directly in the Inspector.

Overriding variables in BTPlayer
--------------------------------

Each :ref:`BTPlayer<class_BTPlayer>` node also has a "Blackboard Plan" property,
providing the ability to override values of the BehaviorTree's blackboard variables.
These overrides are specific to the BTPlayer's scene
and do not impact other scenes using the same :ref:`BehaviorTree<class_BehaviorTree>`.
To modify these values:

1. Select the BTPlayer node in the scene tree.
2. In the Inspector, locate the "Blackboard Plan" property.
3. Override the desired values to tailor the blackboard variables for the specific scene.

Task parameters
---------------

In some cases, it can be beneficial to allow behavior tree tasks to export parameters
that can either be **bound to a blackboard variable or specified directly** by the user.
For this purpose, LimboAI provides special parameter types that begin with "BB",
such as :ref:`BBInt<class_BBInt>`, :ref:`BBBool<class_BBBool>`, :ref:`BBString<class_BBString>`,
:ref:`BBFloat<class_BBFloat>`, :ref:`BBNode<class_BBNode>`, and more.
For a complete list, please refer to the :ref:`BBParam<class_BBParam>` class reference.

Usage example:

.. code:: gdscript

    extends BTAction

    @export var speed: BBFloat

    func _tick(delta: float) -> Status:
        var current_speed: float = speed.get_value(scene_root, blackboard, 0.0)
        ...

Connecting variables in BTs to HSMs: Variable mapping
-----------------------------------------------------

Each :ref:`BTState <class_BTState>` creates a new blackboard scope for its
BehaviorTree instance. Because BehaviorTrees are reusable resources that can run
in different contexts or on different agents, they do not automatically see
variables defined in the HSM. At runtime, HSM variables are usually accessible
inside the BT via the parent scope blackboard, but the recommended way to access
them is through **mapping**.

.. note::
   To learn more about scopes, see :ref:`Blackboard <class_Blackboard>`.

When both the BehaviorTree and the HSM declare variables in their respective
:ref:`BlackboardPlan <class_BlackboardPlan>` resources, the editor provides a **Mapping**
section in the BlackboardPlan inspector. Mapping is the intended and recommended way to
connect variables between related plans.

Key points about mapping
~~~~~~~~~~~~~~~~~~~~~~~~

* Mapping connects two variables so they behave as a single logical variable at runtime.
* Linked variables are updated immediately in both scopes — they literally share the
  same state in memory (no polling or copying).
* Mapping is explicit: you decide which variables the BT exposes as inputs and outputs.
* Linkage is bidirectional — there is no distinction between input and output.
* Mapping does not create new variables automatically — the variables must already
  exist in both blackboard plans before they can be linked.

Inspector workflow
~~~~~~~~~~~~~~~~~~

1. Define the required variables in each BlackboardPlan (HSM plan and BT plan, for example).
2. Select the :ref:`BTState <class_BTState>` node in the scene or a :ref:`BTSubtree <class_BTSubtree>`
   inside a behavior tree.
3. In the Inspector, select the Blackboard Plan property.
4. In the BlackboardPlan resource inspector, locate the **Mapping** section.
5. Create mappings by pairing variables (``BT variable ↔ parent variable``).
6. On startup the mappings are applied, and the variables are automatically linked
   for the running instance.

Programmatic alternative: linking variables in code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can also link variables directly in code using :ref:`Blackboard.link_var <class_Blackboard_method_link_var>`:

.. code-block:: gdscript

   # Example: programmatically link a BT blackboard variable to an HSM blackboard variable
   # Assume `hsm` is a LimboHSM instance and `bt_state` is a BTState instance
   var hsm_bb := hsm.get_blackboard()
   var bt_bb := bt_state.get_blackboard()

   # NOTE: variable names do not need to match
   bt_bb.link_var("target_pos", hsm_bb, "target_pos")

Best practices
~~~~~~~~~~~~~~

* Declare all BT dependencies in the BT’s BlackboardPlan instead of relying on parent scopes.
* Use **Mapping** to explicitly declare which variables should be shared with the HSM.
* Prefer the inspector-based Mapping workflow for clarity and editor support; use
  ``link_var`` only when runtime or programmatic setup is required.
* Avoid writing to ``blackboard.get_parent()`` from inside BT tasks unless you have
  a very specific reason and accept the coupling it introduces.

Advanced topic: Blackboard scopes
---------------------------------

The :ref:`Blackboard<class_Blackboard>` in LimboAI can act as a parent scope
for another :ref:`Blackboard<class_Blackboard>`.
This means that if a specific variable is not found in the active scope,
the system will look in the parent :ref:`Blackboard<class_Blackboard>` to find it.
This creates a "blackboard scope chain," where each :ref:`Blackboard<class_Blackboard>` can have its own parent scope,
and there is no limit to how many blackboards can be in this chain.
It's important to note that the :ref:`Blackboard<class_Blackboard>` doesn't modify values in the parent scopes.

Scopes are created automatically to prevent naming collisions between contextually separate environments:

- Within :ref:`BTNewScope<class_BTNewScope>`.
- Under :ref:`BTSubtree<class_BTSubtree>` decorators.
- With :ref:`LimboState<class_LimboState>` that have a non-empty blackboard plan defined.
- Under :ref:`LimboHSM<class_LimboHSM>` nodes: A new scope is created at the root level,
  and each :ref:`BTState<class_BTState>` child also receives its own separate scope.

Sharing data between several agents
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The blackboard scope mechanism can also be used for sharing data between several agents.
In the following example, we have a group of agents, and we want to share a common target between them:

.. code:: gdscript

    extends BTAction

    @export var group_target_var: StringName = &"group_target"

    func _tick(delta: float) -> Status:
        if not blackboard.has_var(group_target_var):
            var new_target: Node = acquire_target()
            # Set common target shared between agents in a group:
            blackboard.top().set_var(group_target_var, new_target)

        # Access common target shared between agents in a group:
        var target: Node = blackboard.get_var(group_target_var)


In this example, :ref:`blackboard.top()<class_Blackboard_method_top>` accesses the root scope of the
:ref:`Blackboard<class_Blackboard>` chain.
We assign that scope to each agent in a group through code:

.. code:: gdscript

    class_name AgentGroup
    extends Node2D
    ## AgentGroup node: Manages the shared Blackboard for agents in a group.
    ## Children of this node are assumed to be agents that belong to a common group.
    ## This implementation assumes that each agent has a "BTPlayer" node for AI.

    @export var blackboard_plan: BlackboardPlan

    var shared_scope: Blackboard

    func _ready() -> void:
        if blackboard_plan == null:
            shared_scope = Blackboard.new()
        else:
            shared_scope = blackboard_plan.create_blackboard()

        for child in get_children():
            var bt_player: BTPlayer = child.find_child("BTPlayer")
            if is_instance_valid(bt_player):
                bt_player.blackboard.set_parent(shared_scope)

In conclusion, the :ref:`Blackboard<class_Blackboard>` scope chain not only
prevents naming conflicts that can occur between state machines, behavior trees, and sub-trees,
but it can also be used to share data between several agents.
