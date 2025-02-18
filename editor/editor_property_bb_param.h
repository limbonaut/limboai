/**
 * editor_property_bb_param.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#ifndef EDITOR_PROPERTY_BB_PARAM_H
#define EDITOR_PROPERTY_BB_PARAM_H

#include "../compat/forward_decl.h"

#ifdef LIMBOAI_MODULE
#include "editor/editor_inspector.h"
#endif // ! LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_property.hpp>
using namespace godot;
#endif // ! LIMBOAI_GDEXTENSION

class EditorPropertyVariableName;
class BBParam;
class BlackboardPlan;
GODOT_FORWARD_DECLARATIONS()
class MarginContainer;
class MenuButton;
class HBoxContainer;
ENDOF_FORWARD_DECLARATIONS()

class EditorPropertyBBParam : public EditorProperty {
	GDCLASS(EditorPropertyBBParam, EditorProperty);

private:
	const int ID_BIND_VAR = 1000;

	bool initialized = false;

	Ref<BlackboardPlan> plan;
	StringName param_type;
	PropertyHint property_hint = PROPERTY_HINT_NONE;

	HBoxContainer *hbox = nullptr;
	MarginContainer *bottom_container = nullptr;
	HBoxContainer *editor_hbox = nullptr;
	EditorProperty *value_editor = nullptr;
	EditorPropertyVariableName *variable_editor = nullptr;
	MenuButton *type_choice = nullptr;

	Ref<BBParam> _get_edited_param();

	void _create_value_editor(Object *p_object, const String &p_property, Variant::Type p_type);
	void _remove_value_editor();

	void _value_edited(const String &p_property, Variant p_value, const String &p_name = "", bool p_changing = false);
	void _variable_edited(const String &p_property, Variant p_value, const String &p_name = "", bool p_changing = false);
	void _type_selected(int p_index);

protected:
	static void _bind_methods() {}

	void _notification(int p_what);

public:
#ifdef LIMBOAI_MODULE
	virtual void update_property() override;
#elif LIMBOAI_GDEXTENSION
	virtual void _update_property() override;
#endif
	void setup(PropertyHint p_hint, const String &p_hint_text, const Ref<BlackboardPlan> &p_plan);

	EditorPropertyBBParam();
};

class EditorInspectorPluginBBParam : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginBBParam, EditorInspectorPlugin);

private:
	Callable plan_getter;

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

	void set_plan_getter(const Callable &p_getter) { plan_getter = p_getter; }
};

#endif // ! EDITOR_PROPERTY_BB_PARAM_H

#endif // ! TOOLS_ENABLED
