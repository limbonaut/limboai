.. _accessing_nodes:

Accessing nodes in the scene tree
=================================

There are several ways to access nodes in the agent's scene tree (agent is the owner of :ref:`BTPlayer<class_BTPlayer>` node):

1. You can export a :ref:`BBNode<class_BBNode>` variable:

.. code:: gdscript

   @export var cast: BBNode

   func _tick(delta) -> Status:
       var node: ShapeCast3D = cast.get_value(agent, blackboard)

2. You can export a ``NodePath``

.. code:: gdscript

   @export var cast_path: NodePath
   var _shape_cast: ShapeCast3D

   func _setup() -> void:
       _shape_cast = agent.get_node(cast_path)

3. You can :ref:`create a blackboard variable<editing_plan>` in the editor with the type ``NodePath``
and point it to the proper node in the :ref:`BTPlayer<class_BTPlayer>` blackboard plan.

.. code:: gdscript

   extends BTCondition

   @export var shape_var: String = "shape_cast"

   func _tick(delta) -> Status:
       var shape_cast: ShapeCast3D = blackboard.get_var(shape_var)

The property :ref:`BTPlayer.prefetch_nodepath_vars<class_BTPlayer_property_prefetch_nodepath_vars>` should be set to ``true``.
