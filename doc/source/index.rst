LimboAI Documentation
=====================

**LimboAI** is an open-source C++ module for **Godot Engine 4** providing a combination of
**Behavior Trees** and **State Machines** for crafting your game’s AI. It comes with a
behavior tree editor, built-in documentation, visual debugger, and more! While
it is implemented in C++, it fully supports GDScript for :ref:`creating your own tasks <custom_tasks>`
and states. The full list of features is available on the
`LimboAI GitHub <https://github.com/limbonaut/limboai#features>`_ page.

.. SCREENSHOT

**Behavior Trees** are powerful hierarchical structures used to model and control the behavior
of agents in a game (e.g., characters, enemies, entities). They are designed to
make it easier to create complex and highly modular behaviors for your games.
To learn more about behavior trees, check out :ref:`introduction`.

**Hierarchical State Machines** are finite state machines that allow any state to host their own
sub-state machine. This allows you to tackle your AI's state and transition complexity by breaking down
one big state machine into multiple smaller ones.

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: Getting started

   getting-started/getting-limboai
   getting-started/c-sharp
   getting-started/contributing

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: Behavior Trees

   behavior-trees/introduction
   behavior-trees/create-tree
   behavior-trees/custom-tasks
   behavior-trees/using-blackboard
   behavior-trees/accessing-nodes

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: Hierarchical State MachineS

   hierarchical-state-machines/create-hsm

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: Class reference
   :glob:

   classes/featured-classes
   classes/class_*
