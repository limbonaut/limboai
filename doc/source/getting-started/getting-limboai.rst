Getting LimboAI
===============

LimboAI can be used as either a C++ module or as a GDExtension shared library.
There are some differences between the two. In short, GDExtension version is more
convenient to use but somewhat limited in features. The module version provides better editor
experience and is slightly more performant, but it requires using custom engine builds including the export templates.
Whichever you choose to use, your project will stay compatible with both and you can switch from one to
the other any time.

Choose the version you'd like to use. If you're unsure, start with the GDExtension version.
You can change your decision at any time - both versions are fully compatible.

Get GDExtension version
------------------------

Precompiled builds are available on the official
`LimboAI GitHub <https://github.com/limbonaut/limboai#getting-limboai>`_ page,
and in the Asset Library.

GDExtension is the most convenient way of using the LimboAI plugin, but it comes
with certain limitations:

* Documentation tooltips are not available.
* Handy :ref:`class_BBParam` property editor is not available in the extension due to dependencies on the engine classes that are not available in the Godot API.

    **🛈 See also:** `What is GDExtension? <https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/what_is_gdextension.html#what-is-gdextension>`_

Installation instructions:

1. Make sure you're using the latest stable version of the Godot editor.
2. Create a new project for your experiments with LimboAI.
3. In Godot, click AssetLib tab at the top of the screen and search for LimboAI. Download it. LimboAI plugin will be downloaded with the demo project files. Don't mind the errors printed at this point, this is due to the extension library not being loaded just yet.
4. Reload your project with `Project -> Reload project`. There shouldn't be any errors printed now.
5. In the project files, locate a scene file called `showcase.tscn` and run it. It's the demo's entry point.

Get module version
-------------------

Precompiled builds are available on the official
`LimboAI GitHub <https://github.com/limbonaut/limboai#getting-limboai>`_ page.

Installation instructions:

1. In `GitHub releases <https://github.com/limbonaut/limboai/releases/>`_, download the latest pre-compiled release build for your platform.
2. Download the demo project archive from the same release.
3. Extract the pre-compiled editor and the demo project files.
4. Launch the pre-compiled editor binary, import and open the demo project.
5. Run the project.

**Important**: To export your game using the module version of LimboAI, make sure to use the pre-compiled export templates included in the same GitHub release build.
