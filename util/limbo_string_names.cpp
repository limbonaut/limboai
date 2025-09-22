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

LimboStringNames *LimboStringNames::singleton = nullptr;

LimboStringNames::LimboStringNames() {
	_generate_name = StringName("_generate_name");
	_initialize_bt = StringName("_initialize_bt");
	_param_type = StringName("_param_type");
	_replace_task = StringName("_replace_task");
	_update_task_tree = StringName("_update_task_tree");
	_weight_ = StringName("_weight_");
	accent_color = StringName("accent_color");
	ActionCopy = StringName("ActionCopy");
	ActionCut = StringName("ActionCut");
	ActionPaste = StringName("ActionPaste");
	active_state_changed = StringName("active_state_changed");
	Add = StringName("Add");
	add_child = StringName("add_child");
	add_child_at_index = StringName("add_child_at_index");
	AnimationFilter = StringName("AnimationFilter");
	BBParam = StringName("BBParam");
	BBString = StringName("BBString");
	behavior_tree_finished = StringName("behavior_tree_finished");
	bold = StringName("bold");
	branch_changed = StringName("branch_changed");
	button_down = StringName("button_down");
	button_up = StringName("button_up");
	call_deferred = StringName("call_deferred");
	changed = StringName("changed");
	class_icon_size = StringName("class_icon_size");
	Clear = StringName("Clear");
	Close = StringName("Close");
	dark_color_2 = StringName("dark_color_2");
	Debug = StringName("Debug");
	disabled_font_color = StringName("disabled_font_color");
	doc_italic = StringName("doc_italic");
	draw = StringName("draw");
	Duplicate = StringName("Duplicate");
	Edit = StringName("Edit");
	EditAddRemove = StringName("EditAddRemove");
	Editor = StringName("Editor");
	EditorFonts = StringName("EditorFonts");
	EditorIcons = StringName("EditorIcons");
	EditorStyles = StringName("EditorStyles");
	emit_signal = StringName("emit_signal");
	entered = StringName("entered");
	error_value = StringName("error_value");
	EVENT_FAILURE = StringName("failure");
	EVENT_SUCCESS = StringName("success");
	exited = StringName("exited");
	ExternalLink = StringName("ExternalLink");
	favorite_tasks_changed = StringName("favorite_tasks_changed");
	Favorites = StringName("Favorites");
	FlatButton = StringName("FlatButton");
	Focus = StringName("Focus");
	focus_exited = StringName("focus_exited");
	font = StringName("font");
	font_color = StringName("font_color");
	font_size = StringName("font_size");
	freed = StringName("freed");
	gui_input = StringName("gui_input");
	GuiOptionArrow = StringName("GuiOptionArrow");
	GuiTabMenuHl = StringName("GuiTabMenuHl");
	GuiTreeArrowDown = StringName("GuiTreeArrowDown");
	GuiTreeArrowRight = StringName("GuiTreeArrowRight");
	h_separation = StringName("h_separation");
	HeaderSmall = StringName("HeaderSmall");
	Help = StringName("Help");
	icon_max_width = StringName("icon_max_width");
	id_pressed = StringName("id_pressed");
	Info = StringName("Info");
	item_collapsed = StringName("item_collapsed");
	item_selected = StringName("item_selected");
	LimboExtraVariable = StringName("LimboExtraVariable");
	LimboVarAdd = StringName("LimboVarAdd");
	LimboVarEmpty = StringName("LimboVarEmpty");
	LimboVarError = StringName("LimboVarError");
	LimboVarExists = StringName("LimboVarExists");
	LimboVarNotFound = StringName("LimboVarNotFound");
	LimboVarPrivate = StringName("LimboVarPrivate");
	LineEdit = StringName("LineEdit");
	Load = StringName("Load");
	managed = StringName("managed");
	mode_changed = StringName("mode_changed");
	mouse_entered = StringName("mouse_entered");
	mouse_exited = StringName("mouse_exited");
	MoveDown = StringName("MoveDown");
	MoveUp = StringName("MoveUp");
	New = StringName("New");
	NewRoot = StringName("NewRoot");
	NodeWarning = StringName("NodeWarning");
	NonFavorite = StringName("NonFavorite");
	normal = StringName("normal");
	panel = StringName("panel");
	plan_changed = StringName("plan_changed");
	popup_hide = StringName("popup_hide");
	pressed = StringName("pressed");
	probability_clicked = StringName("probability_clicked");
	property_changed = StringName("property_changed");
	ready = StringName("ready");
	Reload = StringName("Reload");
	Remove = StringName("Remove");
	remove_child = StringName("remove_child");
	Rename = StringName("Rename");
	request_open_in_screen = StringName("request_open_in_screen");
	rmb_pressed = StringName("rmb_pressed");
	Save = StringName("Save");
	saved_value = StringName("saved_value");
	Script = StringName("Script");
	ScriptCreate = StringName("ScriptCreate");
	Search = StringName("Search");
	separation = StringName("separation");
	set_custom_name = StringName("set_custom_name");
	_set_enabled = StringName("_set_enabled");
	set_root_task = StringName("set_root_task");
	set_v_scroll = StringName("set_v_scroll");
	set_visible = StringName("set_visible");
	setup = StringName("setup");
	started = StringName("started");
	StatusWarning = StringName("StatusWarning");
	stopped = StringName("stopped");
	task_activated = StringName("task_activated");
	task_button_pressed = StringName("task_button_pressed");
	task_button_rmb = StringName("task_button_rmb");
	task_meta = StringName("task_meta");
	task_selected = StringName("task_selected");
	tasks_dragged = StringName("tasks_dragged");
	text_changed = StringName("text_changed");
	text_submitted = StringName("text_submitted");
	timeout = StringName("timeout");
	toggled = StringName("toggled");
	Tools = StringName("Tools");
	Tree = StringName("Tree");
	TripleBar = StringName("TripleBar");
	update_mode = StringName("update_mode");
	updated = StringName("updated");
	variable = StringName("variable");
	visibility_changed = StringName("visibility_changed");
	window_visibility_changed = StringName("window_visibility_changed");

	repeat_forever = String::utf8("Repeat ∞");
	output_var_prefix = String::utf8("➜");

	node_pp = NodePath("..");
}
