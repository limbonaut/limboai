.. _gdextension:

Using GDExtension
=================

    **🛈 See also:** `What is GDExtension? <https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/what_is_gdextension.html#what-is-gdextension>`_

LimboAI can be used as either a C++ module or as a GDExtension shared library.
The module version is the most feature-full and slightly more performant, but
it requires using custom engine builds including the export templates.

    **🛈 Note:** Precompiled builds are available on the official
    `LimboAI GitHub <https://github.com/limbonaut/limboai#getting-limboai>`_ page.

GDExtension version is more convenient to use, as you don't need a custom engine
build. You can simply download the extension and put it inside your project.
However, it has certain limitations, described in detail in the next section.

Whichever you choose to use, remember, your project will stay compatible with
both and you can transition from one to the other any time.


Limitations of the GDExtension version
--------------------------------------

GDExtension is the most convenient way of using the LimboAI plugin, but it comes
with certain limitations.

The biggest one is that marking methods as virtual for scripting is not
currently possible in godot-cpp. We use these methods to allow creating custom
behavior tree tasks in GDScript.
Due to a workaround we employ, the editor will complain about native
methods being overridden. And by default, such warnings are treated as errors.
You have two options...

In your Project Settings, you can edit ``Debug -> GDScript -> Warnings -> Native Methods Override``
and set it to either ``Warn`` or ``Ignore``.
Those settings are hidden, unless ``Advanced Settings`` toggle is switched on!

Alternatively, in your custom tasks, you can add specific instructions for
the parser to ignore the warning. For example:

.. code:: gdscript

    # The following line instructs parser to ignore the warning:
    @warning_ignore("native_method_override")
    func _tick(p_delta: float) -> Status:
        return SUCCESS

You would have to do that for each overridden method in your custom tasks.
It's up to you which option you prefer. Personally, I'd set it to ``Warn`` and
add ignores only if it gets overwhelming. Hopefully, this will be fixed in the
future releases of Godot!

**Other GDExtension limitations**

* In-editor documentation is not available. The plugin will open online documentation instead when requested.
* Documentation tooltips are not available.
* Handy :ref:`class_BBParam` property editor is not available in the extension due to dependencies on the engine classes that are not available in the Godot API.
