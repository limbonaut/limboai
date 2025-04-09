/**
 * limbo_string_names.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "limbo_string_names.h"

#ifdef LIMBOAI_MODULE
#define SN(m_arg) (StaticCString::create(m_arg))
#endif // ! LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#define SN(m_arg) (StringName(m_arg))
#endif // ! LIMBOAI_GDEXTENSION

LimboStringNames *LimboStringNames::singleton = nullptr;

LimboStringNames::LimboStringNames() {
	_generate_name = SN("_generate_name");
	_param_type = SN("_param_type");
	_replace_task = SN("_replace_task");
	_update_task_tree = SN("_update_task_tree");
	_weight_ = SN("_weight_");
	accent_color = SN("accent_color");
	ActionCopy = SN("ActionCopy");
	ActionCut = SN("ActionCut");
	ActionPaste = SN("ActionPaste");
	active_state_changed = SN("active_state_changed");
	Add = SN("Add");
	add_child = SN("add_child");
	add_child_at_index = SN("add_child_at_index");
	AnimationFilter = SN("AnimationFilter");
	BBParam = SN("BBParam");
	BBString = SN("BBString");
	behavior_tree_finished = SN("behavior_tree_finished");
	bold = SN("bold");
	branch_changed = SN("branch_changed");
	button_down = SN("button_down");
	button_up = SN("button_up");
	call_deferred = SN("call_deferred");
	changed = SN("changed");
	class_icon_size = SN("class_icon_size");
	Clear = SN("Clear");
	Close = SN("Close");
	dark_color_2 = SN("dark_color_2");
	Debug = SN("Debug");
	disabled_font_color = SN("disabled_font_color");
	doc_italic = SN("doc_italic");
	draw = SN("draw");
	Duplicate = SN("Duplicate");
	Edit = SN("Edit");
	EditAddRemove = SN("EditAddRemove");
	Editor = SN("Editor");
	EditorFonts = SN("EditorFonts");
	EditorIcons = SN("EditorIcons");
	EditorStyles = SN("EditorStyles");
	emit_signal = SN("emit_signal");
	entered = SN("entered");
	error_value = SN("error_value");
	EVENT_FAILURE = SN("failure");
	EVENT_SUCCESS = SN("success");
	exited = SN("exited");
	ExternalLink = SN("ExternalLink");
	favorite_tasks_changed = SN("favorite_tasks_changed");
	Favorites = SN("Favorites");
	FlatButton = SN("FlatButton");
	Focus = SN("Focus");
	focus_exited = SN("focus_exited");
	font = SN("font");
	font_color = SN("font_color");
	font_size = SN("font_size");
	freed = SN("freed");
	gui_input = SN("gui_input");
	GuiOptionArrow = SN("GuiOptionArrow");
	GuiTabMenuHl = SN("GuiTabMenuHl");
	GuiTreeArrowDown = SN("GuiTreeArrowDown");
	GuiTreeArrowRight = SN("GuiTreeArrowRight");
	h_separation = SN("h_separation");
	HeaderSmall = SN("HeaderSmall");
	Help = SN("Help");
	icon_max_width = SN("icon_max_width");
	id_pressed = SN("id_pressed");
	Info = SN("Info");
	item_collapsed = SN("item_collapsed");
	item_selected = SN("item_selected");
	LimboExtraVariable = SN("LimboExtraVariable");
	LimboVarAdd = SN("LimboVarAdd");
	LimboVarEmpty = SN("LimboVarEmpty");
	LimboVarError = SN("LimboVarError");
	LimboVarExists = SN("LimboVarExists");
	LimboVarNotFound = SN("LimboVarNotFound");
	LimboVarPrivate = SN("LimboVarPrivate");
	LineEdit = SN("LineEdit");
	Load = SN("Load");
	managed = SN("managed");
	mode_changed = SN("mode_changed");
	mouse_entered = SN("mouse_entered");
	mouse_exited = SN("mouse_exited");
	MoveDown = SN("MoveDown");
	MoveUp = SN("MoveUp");
	New = SN("New");
	NewRoot = SN("NewRoot");
	NodeWarning = SN("NodeWarning");
	NonFavorite = SN("NonFavorite");
	normal = SN("normal");
	panel = SN("panel");
	plan_changed = SN("plan_changed");
	popup_hide = SN("popup_hide");
	pressed = SN("pressed");
	probability_clicked = SN("probability_clicked");
	property_changed = SN("property_changed");
	Reload = SN("Reload");
	Remove = SN("Remove");
	remove_child = SN("remove_child");
	Rename = SN("Rename");
	request_open_in_screen = SN("request_open_in_screen");
	rmb_pressed = SN("rmb_pressed");
	Save = SN("Save");
	saved_value = SN("saved_value");
	Script = SN("Script");
	ScriptCreate = SN("ScriptCreate");
	Search = SN("Search");
	separation = SN("separation");
	set_custom_name = SN("set_custom_name");
	_set_enabled = SN("_set_enabled");
	set_root_task = SN("set_root_task");
	set_v_scroll = SN("set_v_scroll");
	set_visible = SN("set_visible");
	setup = SN("setup");
	started = SN("started");
	StatusWarning = SN("StatusWarning");
	stopped = SN("stopped");
	task_activated = SN("task_activated");
	task_button_pressed = SN("task_button_pressed");
	task_button_rmb = SN("task_button_rmb");
	task_meta = SN("task_meta");
	task_selected = SN("task_selected");
	tasks_dragged = SN("tasks_dragged");
	text_changed = SN("text_changed");
	text_submitted = SN("text_submitted");
	timeout = SN("timeout");
	toggled = SN("toggled");
	Tools = SN("Tools");
	Tree = SN("Tree");
	TripleBar = SN("TripleBar");
	update_mode = SN("update_mode");
	updated = SN("updated");
	variable = SN("variable");
	visibility_changed = SN("visibility_changed");
	window_visibility_changed = SN("window_visibility_changed");

	repeat_forever.parse_utf8("Repeat ∞");
	output_var_prefix.parse_utf8("➜");

	node_pp = NodePath("..");
}
