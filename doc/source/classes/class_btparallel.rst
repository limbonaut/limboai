:github_url: hide

.. DO NOT EDIT THIS FILE!!!
.. Generated automatically from Godot engine sources.
.. Generator: https://github.com/godotengine/godot/tree/4.3/doc/tools/make_rst.py.
.. XML source: https://github.com/godotengine/godot/tree/4.3/modules/limboai/doc_classes/BTParallel.xml.

.. _class_BTParallel:

BTParallel
==========

**Inherits:** :ref:`BTComposite<class_BTComposite>` **<** :ref:`BTTask<class_BTTask>` **<** :ref:`BT<class_BT>`

BT composite that executes all of its child tasks simultaneously.

.. rst-class:: classref-introduction-group

Description
-----------

BTParallel executes all of its child tasks simultaneously. Note that BTParallel doesn't involve multithreading. It processes each task sequentially, from first to last, in the same tick before returning a result. If one of the abort criterea is met, any tasks currently ``RUNNING`` will be terminated, and the result will be either ``FAILURE`` or ``SUCCESS``. The :ref:`num_failures_required<class_BTParallel_property_num_failures_required>` determines when BTParallel fails and :ref:`num_successes_required<class_BTParallel_property_num_successes_required>` when it succeeds. When both are fullfilled, it gives priority to :ref:`num_failures_required<class_BTParallel_property_num_failures_required>`.

If set to :ref:`repeat<class_BTParallel_property_repeat>`, all child tasks will be re-executed each tick, regardless of whether they previously resulted in ``SUCCESS`` or ``FAILURE``.

Returns ``FAILURE`` when the required number of child tasks result in ``FAILURE``. When :ref:`repeat<class_BTParallel_property_repeat>` is set to ``false``, if none of the criteria were met and all child tasks resulted in either ``SUCCESS`` or ``FAILURE``, BTParallel will return ``FAILURE``.

Returns ``SUCCESS`` when the required number of child tasks result in ``SUCCESS``.

Returns ``RUNNING`` if none of the criterea were fulfilled, and either :ref:`repeat<class_BTParallel_property_repeat>` is set to ``true`` or a child task resulted in ``RUNNING``.

.. rst-class:: classref-reftable-group

Properties
----------

.. table::
   :widths: auto

   +----------+---------------------------------------------------------------------------------+-----------+
   | ``int``  | :ref:`num_failures_required<class_BTParallel_property_num_failures_required>`   | ``1``     |
   +----------+---------------------------------------------------------------------------------+-----------+
   | ``int``  | :ref:`num_successes_required<class_BTParallel_property_num_successes_required>` | ``1``     |
   +----------+---------------------------------------------------------------------------------+-----------+
   | ``bool`` | :ref:`repeat<class_BTParallel_property_repeat>`                                 | ``false`` |
   +----------+---------------------------------------------------------------------------------+-----------+

.. rst-class:: classref-section-separator

----

.. rst-class:: classref-descriptions-group

Property Descriptions
---------------------

.. _class_BTParallel_property_num_failures_required:

.. rst-class:: classref-property

``int`` **num_failures_required** = ``1`` :ref:`🔗<class_BTParallel_property_num_failures_required>`

.. rst-class:: classref-property-setget

- |void| **set_num_failures_required**\ (\ value\: ``int``\ )
- ``int`` **get_num_failures_required**\ (\ )

If the specified number of child tasks return ``FAILURE``, BTParallel will also return ``FAILURE``.

.. rst-class:: classref-item-separator

----

.. _class_BTParallel_property_num_successes_required:

.. rst-class:: classref-property

``int`` **num_successes_required** = ``1`` :ref:`🔗<class_BTParallel_property_num_successes_required>`

.. rst-class:: classref-property-setget

- |void| **set_num_successes_required**\ (\ value\: ``int``\ )
- ``int`` **get_num_successes_required**\ (\ )

If the specified number of child tasks return ``SUCCESS``, BTParallel will also return ``SUCCESS``.

.. rst-class:: classref-item-separator

----

.. _class_BTParallel_property_repeat:

.. rst-class:: classref-property

``bool`` **repeat** = ``false`` :ref:`🔗<class_BTParallel_property_repeat>`

.. rst-class:: classref-property-setget

- |void| **set_repeat**\ (\ value\: ``bool``\ )
- ``bool`` **get_repeat**\ (\ )

When ``true``, the child tasks will be executed again, regardless of whether they previously resulted in a ``SUCCESS`` or ``FAILURE``.

When ``false``, if none of the criteria were met, and all child tasks resulted in a ``SUCCESS`` or ``FAILURE``, BTParallel will return ``FAILURE``.

.. |virtual| replace:: :abbr:`virtual (This method should typically be overridden by the user to have any effect.)`
.. |const| replace:: :abbr:`const (This method has no side effects. It doesn't modify any of the instance's member variables.)`
.. |vararg| replace:: :abbr:`vararg (This method accepts any number of arguments after the ones described here.)`
.. |constructor| replace:: :abbr:`constructor (This method is used to construct a type.)`
.. |static| replace:: :abbr:`static (This method doesn't need an instance to be called, so it can be called directly using the class name.)`
.. |operator| replace:: :abbr:`operator (This method describes a valid operator to use with this type as left-hand operand.)`
.. |bitfield| replace:: :abbr:`BitField (This value is an integer composed as a bitmask of the following flags.)`
.. |void| replace:: :abbr:`void (No return value.)`
