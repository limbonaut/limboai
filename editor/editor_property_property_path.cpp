/**
 * editor_property_path.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "editor_property_property_path.h"

#include "../compat/translation.h"
#include "../util/limbo_string_names.h"

#ifdef LIMBOAI_MODULE
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#include "servers/display_server.h"
#endif

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#endif // LIMBOAI_MODULE

#ifdef TOOLS_ENABLED

namespace {

Node *_get_base_node(Object *p_edited_object, SceneTree *p_scene_tree) {
	Node *base_node = Object::cast_to<Node>(p_edited_object);
	if (!base_node) {
		base_node = Object::cast_to<Node>(EditorInterface::get_singleton()->get_inspector()->get_edited_object());
	}
	if (!base_node) {
		base_node = p_scene_tree->get_edited_scene_root();
	}
	return base_node;
}

} // unnamed namespace

Node *EditorPropertyPropertyPath::_get_selected_node() {
	ERR_FAIL_NULL_V(get_edited_object(), nullptr);

	NodePath path = get_edited_object()->get(get_edited_property());
	if (path.is_empty()) {
		return nullptr;
	}

	Node *base_node = _get_base_node(get_edited_object(), get_tree());
	ERR_FAIL_NULL_V(base_node, nullptr);
	Node *selected_node = base_node->get_node_or_null(path);
	return selected_node;
}

void EditorPropertyPropertyPath::_action_selected(int p_idx) {
	switch (p_idx) {
		case ACTION_CLEAR: {
			emit_changed(get_edited_property(), NodePath());
		} break;
		case ACTION_COPY: {
			DisplayServer::get_singleton()->clipboard_set(get_edited_object()->get(get_edited_property()));
		} break;
		case ACTION_EDIT: {
			assign_button->hide();
			action_menu->hide();
			path_edit->show();
			path_edit->set_text(get_edited_object()->get(get_edited_property()));
			path_edit->grab_focus();
		} break;
		case ACTION_SELECT: {
			Node *selected_node = _get_selected_node();
			if (selected_node) {
				EditorInterface::get_singleton()->get_selection()->clear();
				EditorInterface::get_singleton()->get_selection()->add_node(selected_node);
			}
		} break;
	}
}

void EditorPropertyPropertyPath::_accept_text() {
	path_edit->hide();
	assign_button->show();
	action_menu->show();
	emit_changed(get_edited_property(), path_edit->get_text());
}

void EditorPropertyPropertyPath::_property_selected(const NodePath &p_property_path, const NodePath &p_node_path) {
	if (p_property_path.is_empty()) {
		return;
	}
	Node *base_node = _get_base_node(get_edited_object(), get_tree());
	ERR_FAIL_NULL(base_node);
	Node *selected_node = get_tree()->get_edited_scene_root()->get_node_or_null(p_node_path);
	ERR_FAIL_NULL(selected_node);
	NodePath path = String(base_node->get_path_to(selected_node)) + String(p_property_path);

	emit_changed(get_edited_property(), path);
	update_property();
}

void EditorPropertyPropertyPath::_node_selected(const NodePath &p_path) {
	if (p_path.is_empty()) {
		return;
	}
	Node *selected_node = get_tree()->get_edited_scene_root()->get_node_or_null(p_path);
	ERR_FAIL_NULL(selected_node);
	EditorInterface::get_singleton()->popup_property_selector(
			selected_node,
			callable_mp(this, &EditorPropertyPropertyPath::_property_selected).bind(p_path),
			valid_types);
}

void EditorPropertyPropertyPath::_choose_property() {
	EditorInterface::get_singleton()->popup_node_selector(callable_mp(this, &EditorPropertyPropertyPath::_node_selected));
}

void EditorPropertyPropertyPath::_set_read_only(bool p_read_only) {
	assign_button->set_disabled(p_read_only);
	action_menu->set_disabled(p_read_only);
}

#ifdef LIMBOAI_MODULE
void EditorPropertyPropertyPath::update_property() {
#elif LIMBOAI_GDEXTENSION
void EditorPropertyPropertyPath::_update_property() {
#endif
	NodePath path = get_edited_object()->get(get_edited_property());
	if (path.is_empty()) {
		assign_button->set_text(TTR("Bind..."));
	} else {
		Node *base_node = _get_base_node(get_edited_object(), get_tree());
		ERR_FAIL_NULL(base_node);
		Node *selected_node = base_node->get_node_or_null(path);
		String text;
		if (selected_node) {
			text = (String)selected_node->get_name() +
					":" + (String)path.get_concatenated_subnames();
		} else {
			text = (String)path;
		}
		assign_button->set_text(text);
		assign_button->set_tooltip_text(path);
	}
}

void EditorPropertyPropertyPath::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			action_menu->set_button_icon(get_theme_icon(LW_NAME(GuiTabMenuHl), LW_NAME(EditorIcons)));
			action_menu->get_popup()->set_item_icon(ACTION_CLEAR, get_theme_icon(LW_NAME(Clear), LW_NAME(EditorIcons)));
			action_menu->get_popup()->set_item_icon(ACTION_COPY, get_theme_icon(LW_NAME(ActionCopy), LW_NAME(EditorIcons)));
			action_menu->get_popup()->set_item_icon(ACTION_EDIT, get_theme_icon(LW_NAME(Edit), LW_NAME(EditorIcons)));
			action_menu->get_popup()->set_item_icon(ACTION_SELECT, get_theme_icon(LW_NAME(ExternalLink), LW_NAME(EditorIcons)));
		} break;
	}
}

void EditorPropertyPropertyPath::setup(const PackedInt32Array &p_valid_types) {
	valid_types = p_valid_types;
}

EditorPropertyPropertyPath::EditorPropertyPropertyPath() {
	HBoxContainer *hb = memnew(HBoxContainer);
	add_child(hb);
	hb->add_theme_constant_override(LW_NAME(separation), 0);

	assign_button = memnew(Button);
	hb->add_child(assign_button);
	assign_button->set_flat(true);
	assign_button->set_text(TTR("Bind..."));
	assign_button->set_clip_text(true);
	assign_button->set_h_size_flags(SIZE_EXPAND_FILL);
	assign_button->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
	assign_button->connect(LW_NAME(pressed), callable_mp(this, &EditorPropertyPropertyPath::_choose_property));

	path_edit = memnew(LineEdit);
	hb->add_child(path_edit);
	path_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	path_edit->connect(LW_NAME(focus_exited), callable_mp(this, &EditorPropertyPropertyPath::_accept_text));
	path_edit->connect(LW_NAME(text_submitted), callable_mp(this, &EditorPropertyPropertyPath::_accept_text).unbind(1));
	path_edit->hide();

	action_menu = memnew(MenuButton);
	action_menu->get_popup()->add_item(TTR("Clear"), ACTION_CLEAR);
	action_menu->get_popup()->add_item(TTR("Copy as Text"), ACTION_COPY);
	action_menu->get_popup()->add_item(TTR("Edit"), ACTION_EDIT);
	action_menu->get_popup()->add_item(TTR("Show Node in Tree"), ACTION_SELECT);
	action_menu->get_popup()->connect(LW_NAME(id_pressed), callable_mp(this, &EditorPropertyPropertyPath::_action_selected));
	hb->add_child(action_menu);
}

//***** EditorInspectorPluginPropertyPath

#ifdef LIMBOAI_MODULE
bool EditorInspectorPluginPropertyPath::can_handle(Object *p_object) {
#elif LIMBOAI_GDEXTENSION
bool EditorInspectorPluginPropertyPath::_can_handle(Object *p_object) const {
#endif
	return true;
}

#ifdef LIMBOAI_MODULE
bool EditorInspectorPluginPropertyPath::parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const BitField<PropertyUsageFlags> p_usage, const bool p_wide) {
#elif LIMBOAI_GDEXTENSION
bool EditorInspectorPluginPropertyPath::_parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const BitField<PropertyUsageFlags> p_usage, const bool p_wide) {
#endif
	if (p_type != Variant::NODE_PATH || p_hint != PROPERTY_HINT_LINK) {
		return false;
	}

	EditorPropertyPropertyPath *ed = memnew(EditorPropertyPropertyPath);

	// Convert the hint text to an array of valid types.
	PackedInt32Array valid_types;
	PackedStringArray type_specifiers = p_hint_text.split(",");
	for (const String &t : type_specifiers) {
		if (t.is_valid_int()) {
			valid_types.append(t.to_int());
		}
	}
	ed->setup(valid_types);

	add_property_editor(p_path, ed);

	return true;
}

#endif // TOOLS_ENABLED
