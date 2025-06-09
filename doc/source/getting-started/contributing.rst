.. _contributing:

Contributing
============

We target the latest stable version of the Godot Engine for development until a
third beta or a release candidate of an upcoming Godot release becomes available.
If you want to contribute to the project, please ensure that you are using the
corresponding Godot version.

We follow the `Godot code style guidelines <https://docs.godotengine.org/en/stable/contributing/development/code_style_guidelines.html#doc-code-style-guidelines>`_.
Please use ``clang-format`` to maintain consistent styling. You can install
``pre-commit`` hooks for Git using ``pre-commit install`` to automate this process.

Please keep the minor versions backward-compatible when submitting pull requests.

We support building LimboAI as a module for the Godot Engine and as a GDExtension library.
Make sure your contribution is compatible with both. Our CI workflow will verify
that your changes can be compiled in both configurations.

Compiling as module of the Godot Engine
---------------------------------------

1. Clone the Godot Engine repository.
2. Switch to the latest stable tag.
3. Clone the LimboAI repository into the ``modules/limboai`` directory.

.. code-block:: bash

   git clone https://github.com/godotengine/godot.git
   git checkout 4.3-stable  # Replace "4.3-stable" with the latest stable tag
   git clone https://github.com/limbonaut/limboai modules/limboai

Consult the `Godot Engine documentation <https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html>`_
for detailed instructions on building the engine.

**Unit tests** can be compiled using the ``tests=yes`` build option. To execute them,
run the compiled Godot binary with the ``--test --tc="*[LimboAI]*"`` command-line options.

Building C# Packages
~~~~~~~~~~~~~~~~~~~~

If you want C# Support for LimboAI when building from source, follow Godot's instructions for building from source with
C# Support: https://docs.godotengine.org/en/stable/contributing/development/compiling/compiling_with_dotnet.html.
LimboAI's classes will be included in the output from that build process.

Compiling as GDExtension library
--------------------------------

You'll need the SCons build tool and a C++ compiler. See also `Compiling <https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html>`_.
Run ``scons target=editor`` to build the plugin library for your current platform.

- SCons will automatically clone the ``godot-cpp`` repository if it doesn't already exist in the ``limboai/godot-cpp`` directory.
- By default, built targets are placed in the demo project: ``demo/addons/limboai/bin/``.

Check ``scons -h`` for other options and targets.

Contributing to the documentation
---------------------------------

Online documentation is created using `Sphinx <https://www.sphinx-doc.org/en/master/>`_.
Source files are located in the ``doc/source`` directory in RST format and can
be built locally with ``sphinx-build``. See the
`Sphinx documentation <https://www.sphinx-doc.org/en/master/tutorial/getting-started.html>`_
for more details.

Class documentation resides in XML files within the ``doc_classes/`` directory.
If you create a new class or modify an existing one, you can run the compiled
Godot binary with the ``--doctool`` option in the root of the Godot source code
to generate the missing XML files or sections within those files in the class documentation.
After editing the XML files, please run the compiled editor binary with the ``--doctool``
option again to update and tidy up the XML files.

Sphinx RST files for the class documentation are generated from
XML files using the Godot script ``make_rst.py`` and stored in the ``doc/source/classes`` directory.
This process is performed using our own script ``scripts/update_rst.sh``. RST files
in ``doc/source/classes`` should not be edited manually.
