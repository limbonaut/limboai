/**
 * editor_property_bb_param.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "editor_property_bb_param.h"

#include "modules/limboai/blackboard/bb_param/bb_param.h"

#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/os/memory.h"
#include "core/string/print_string.h"
#include "editor/editor_inspector.h"
#include "editor/editor_properties.h"
#include "editor/editor_properties_array_dict.h"
#include "editor/editor_properties_vector.h"
#include "editor/editor_settings.h"
#include "scene/gui/base_button.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/line_edit.h"

Ref<BBParam> EditorPropertyBBParam::_get_edited_param() {
	Ref<BBParam> param = get_edited_property_value();
	if (param.is_null()) {
		// Create parameter resource if null.
		param = ClassDB::instantiate(param_type);
		get_edited_object()->set(get_edited_property(), param);
	}
	return param;
}

void EditorPropertyBBParam::_create_value_editor(Variant::Type p_type) {
	if (value_editor) {
		if (value_editor->get_meta(SNAME("_param_type")) == Variant(p_type)) {
			return;
		}
		_remove_value_editor();
	}

	switch (p_type) {
		case Variant::BOOL: {
			value_editor = memnew(EditorPropertyCheck);
		} break;
		case Variant::INT: {
			EditorPropertyInteger *editor = memnew(EditorPropertyInteger);
			editor->setup(-100000, 100000, 1, false, true, true);
			value_editor = editor;
		} break;
		case Variant::FLOAT: {
			EditorPropertyFloat *editor = memnew(EditorPropertyFloat);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true, false, true, true);
			value_editor = editor;
		} break;
		case Variant::STRING: {
			if (property_hint == PROPERTY_HINT_MULTILINE_TEXT) {
				value_editor = memnew(EditorPropertyMultilineText);
			} else {
				value_editor = memnew(EditorPropertyText);
			}
		} break;
		case Variant::VECTOR2: {
			EditorPropertyVector2 *editor = memnew(EditorPropertyVector2);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::VECTOR2I: {
			EditorPropertyVector2i *editor = memnew(EditorPropertyVector2i);
			editor->setup(-100000, 100000);
			value_editor = editor;
		} break;
		case Variant::RECT2: {
			EditorPropertyRect2 *editor = memnew(EditorPropertyRect2);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::RECT2I: {
			EditorPropertyRect2i *editor = memnew(EditorPropertyRect2i);
			editor->setup(-100000, 100000);
			value_editor = editor;
		} break;
		case Variant::VECTOR3: {
			EditorPropertyVector3 *editor = memnew(EditorPropertyVector3);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::VECTOR3I: {
			EditorPropertyVector3i *editor = memnew(EditorPropertyVector3i);
			editor->setup(-100000, 100000);
			value_editor = editor;
		} break;
		case Variant::VECTOR4: {
			EditorPropertyVector4 *editor = memnew(EditorPropertyVector4);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::VECTOR4I: {
			EditorPropertyVector4i *editor = memnew(EditorPropertyVector4i);
			editor->setup(-100000, 100000);
			value_editor = editor;
		} break;
		case Variant::TRANSFORM2D: {
			EditorPropertyTransform2D *editor = memnew(EditorPropertyTransform2D);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::PLANE: {
			EditorPropertyPlane *editor = memnew(EditorPropertyPlane);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::QUATERNION: {
			EditorPropertyQuaternion *editor = memnew(EditorPropertyQuaternion);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::AABB: {
			EditorPropertyAABB *editor = memnew(EditorPropertyAABB);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::BASIS: {
			EditorPropertyBasis *editor = memnew(EditorPropertyBasis);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::TRANSFORM3D: {
			EditorPropertyTransform3D *editor = memnew(EditorPropertyTransform3D);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::PROJECTION: {
			EditorPropertyProjection *editor = memnew(EditorPropertyProjection);
			editor->setup(-100000, 100000, EDITOR_GET("interface/inspector/default_float_step"), true);
			value_editor = editor;
		} break;
		case Variant::COLOR: {
			value_editor = memnew(EditorPropertyColor);
		} break;
		case Variant::STRING_NAME: {
			EditorPropertyText *editor = memnew(EditorPropertyText);
			editor->set_string_name(true);
			value_editor = editor;
		} break;
		case Variant::NODE_PATH: {
			value_editor = memnew(EditorPropertyNodePath);
		} break;
		// case Variant::RID: {
		// } break;
		// case Variant::SIGNAL: {
		// } break;
		// case Variant::CALLABLE: {
		// } break;
		// case Variant::OBJECT: {
		// } break;
		case Variant::DICTIONARY: {
			value_editor = memnew(EditorPropertyDictionary);
		} break;

		case Variant::ARRAY:
		case Variant::PACKED_BYTE_ARRAY:
		case Variant::PACKED_INT32_ARRAY:
		case Variant::PACKED_FLOAT32_ARRAY:
		case Variant::PACKED_INT64_ARRAY:
		case Variant::PACKED_FLOAT64_ARRAY:
		case Variant::PACKED_STRING_ARRAY:
		case Variant::PACKED_VECTOR2_ARRAY:
		case Variant::PACKED_VECTOR3_ARRAY:
		case Variant::PACKED_COLOR_ARRAY: {
			EditorPropertyArray *editor = memnew(EditorPropertyArray);
			editor->setup(p_type);
			value_editor = editor;
		} break;

		default: {
			ERR_PRINT("Unexpected variant type!");
			value_editor = memnew(EditorPropertyNil);
		}
	}
	value_editor->set_name_split_ratio(0.0);
	hbox->add_child(value_editor);
	value_editor->set_h_size_flags(SIZE_EXPAND_FILL);
	value_editor->set_meta(SNAME("_param_type"), p_type);
	value_editor->connect(SNAME("property_changed"), callable_mp(this, &EditorPropertyBBParam::_value_edited));
}

void EditorPropertyBBParam::_remove_value_editor() {
	if (value_editor) {
		hbox->remove_child(value_editor);
		value_editor->queue_free();
		value_editor = nullptr;
	}
}

void EditorPropertyBBParam::_value_edited(const String &p_property, Variant p_value, const String &p_name, bool p_changing) {
	_get_edited_param()->set_saved_value(p_value);
}

void EditorPropertyBBParam::_mode_changed() {
	_get_edited_param()->set_value_source(value_mode->is_pressed() ? BBParam::SAVED_VALUE : BBParam::BLACKBOARD_VAR);
	update_property();
}

void EditorPropertyBBParam::_variable_edited(const String &p_text) {
	_get_edited_param()->set_variable(p_text);
}

void EditorPropertyBBParam::update_property() {
	Ref<BBParam> param = _get_edited_param();
	if (param->get_value_source() == BBParam::BLACKBOARD_VAR) {
		_remove_value_editor();
		variable_edit->set_text(param->get_variable());
		variable_edit->set_editable(true);
		variable_edit->show();
		variable_mode->set_pressed_no_signal(true);
		value_mode->set_pressed_no_signal(false);
	} else {
		variable_edit->hide();
		_create_value_editor(param->get_type());
		value_editor->show();
		value_editor->set_object_and_property(param.ptr(), SNAME("saved_value"));
		value_mode->set_pressed_no_signal(true);
		variable_mode->set_pressed_no_signal(false);
		value_editor->update_property();
	}
}

void EditorPropertyBBParam::setup(PropertyHint p_hint, const String &p_hint_text) {
	param_type = p_hint_text;
	property_hint = p_hint;
}

void EditorPropertyBBParam::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			value_mode->set_icon(get_editor_theme_icon(SNAME("LimboSpecifyValue")));
			variable_mode->set_icon(get_editor_theme_icon(SNAME("BTSetVar")));
		} break;
	}
}

EditorPropertyBBParam::EditorPropertyBBParam() {
	hbox = memnew(HBoxContainer);
	add_child(hbox);

	HBoxContainer *modes = memnew(HBoxContainer);
	hbox->add_child(modes);
	modes->add_theme_constant_override("separation", 0);

	Ref<ButtonGroup> modes_group;
	modes_group.instantiate();

	value_mode = memnew(Button);
	modes->add_child(value_mode);
	value_mode->set_focus_mode(FOCUS_NONE);
	value_mode->set_button_group(modes_group);
	value_mode->set_toggle_mode(true);
	value_mode->set_tooltip_text(TTR("Specify value"));
	value_mode->connect(SNAME("pressed"), callable_mp(this, &EditorPropertyBBParam::_mode_changed));

	variable_mode = memnew(Button);
	modes->add_child(variable_mode);
	variable_mode->set_focus_mode(FOCUS_NONE);
	variable_mode->set_button_group(modes_group);
	variable_mode->set_toggle_mode(true);
	variable_mode->set_tooltip_text(TTR("Bind to a blackboard variable"));
	variable_mode->connect(SNAME("pressed"), callable_mp(this, &EditorPropertyBBParam::_mode_changed));

	variable_edit = memnew(LineEdit);
	hbox->add_child(variable_edit);
	variable_edit->set_placeholder(TTR("Variable"));
	variable_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	variable_edit->connect(SNAME("text_changed"), callable_mp(this, &EditorPropertyBBParam::_variable_edited));

	param_type = SNAME("BBString");
}

//***** EditorInspectorPluginBBParam

bool EditorInspectorPluginBBParam::can_handle(Object *p_object) {
	return true; // Handles everything.
}

bool EditorInspectorPluginBBParam::parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const BitField<PropertyUsageFlags> p_usage, const bool p_wide) {
	if (p_hint == PROPERTY_HINT_RESOURCE_TYPE && p_hint_text.begins_with("BB")) {
		// TODO: Add more rigid hint check.
		EditorPropertyBBParam *editor = memnew(EditorPropertyBBParam());
		editor->setup(p_hint, p_hint_text);
		add_property_editor(p_path, editor);
		return true;
	}
	return false;
}
