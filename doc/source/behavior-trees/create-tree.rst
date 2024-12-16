.. _create_tree::

Creating Behavior Trees
=======================

This chapter describes how to create and debug behavior trees.

Add a Behavior Tree to an agent
-------------------------------

Follow these steps to add a behavior tree to a new or existing agent:

1. Make a scene file for your agent, or open an existing scene.
2. Add a :ref:`BTPlayer<class_BTPlayer>` node to your scene.
3. Select :ref:`BTPlayer<class_BTPlayer>`, and create a new behavior tree in the inspector.
4. Optionally, you can save the behavior tree to a file using the property's context menu.
5. Click the behavior tree property to open it in the LimboAI editor.

Debugging Behavior Trees
------------------------

In Godot Engine, follow to "Bottom Panel > Debugger > LimboAI" tab. With the LimboAI debugger,
you can inspect any currently active behavior tree within the running project. The debugger can be detached
from the main editor window, which can be particularly useful if you have a HiDPI or a secondary display.