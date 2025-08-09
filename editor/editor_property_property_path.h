/**
 * editor_property_path.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef EDITOR_PROPERTY_PROPERTY_PATH
#define EDITOR_PROPERTY_PROPERTY_PATH

#ifdef TOOLS_ENABLED

#ifdef LIMBOAI_MODULE
#include "editor/inspector/editor_inspector.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/menu_button.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/menu_button.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

// Specialized property editor for NodePath properties that represent a path to a specific property instead of just a node.
// Handles NodePath properties that have PROPERTY_HINT_LINK.
// Hint string can list the valid Variant types as comma-separated integers.
class EditorPropertyPropertyPath : public EditorProperty {
	GDCLASS(EditorPropertyPropertyPath, EditorProperty);

private:
	enum Action {
		ACTION_CLEAR,
		ACTION_COPY,
		ACTION_EDIT,
		ACTION_SELECT,
	};

	Button *assign_button;
	MenuButton *action_menu;
	LineEdit *path_edit;

	PackedInt32Array valid_types;

	Node *_get_selected_node();
	void _action_selected(int p_idx);
	void _accept_text();
	void _property_selected(const NodePath &p_property_path, const NodePath &p_node_path);
	void _node_selected(const NodePath &p_path);
	void _choose_property();

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	// Note: Needs to be public in GDExtension.
	virtual void _set_read_only(bool p_read_only) override;

#ifdef LIMBOAI_MODULE
	virtual void update_property() override;
#elif LIMBOAI_GDEXTENSION
	virtual void _update_property() override;
#endif

	void setup(const PackedInt32Array &p_valid_types);
	EditorPropertyPropertyPath();
};

class EditorInspectorPluginPropertyPath : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginPropertyPath, EditorInspectorPlugin);

private:
protected:
	static void _bind_methods() {}

public:
#ifdef LIMBOAI_MODULE
	virtual bool can_handle(Object *p_object) override;
	virtual bool parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const BitField<PropertyUsageFlags> p_usage, const bool p_wide = false) override;
#elif LIMBOAI_GDEXTENSION
	virtual bool _can_handle(Object *p_object) const override;
	virtual bool _parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const BitField<PropertyUsageFlags> p_usage, const bool p_wide = false) override;
#endif

	EditorInspectorPluginPropertyPath() = default;
};

#endif // TOOLS_ENABLED

#endif // EDITOR_PROPERTY_PROPERTY_PATH
