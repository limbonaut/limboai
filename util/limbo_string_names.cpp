/**
 * limbo_string_names.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "limbo_string_names.h"
#include "godot_cpp/variant/string_name.hpp"

#ifdef LIMBOAI_MODULE
#define SN(m_arg) (StaticCString::create(m_arg))
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#define SN(m_arg) (StringName(m_arg))
#endif // LIMBOAI_GDEXTENSION

LimboStringNames *LimboStringNames::singleton = nullptr;

LimboStringNames::LimboStringNames() {
	_generate_name = SN("_generate_name");
	_setup = SN("_setup");
	_enter = SN("_enter");
	_exit = SN("_exit");
	_tick = SN("_tick");
	behavior_tree_finished = SN("behavior_tree_finished");
	setup = SN("setup");
	entered = SN("entered");
	exited = SN("exited");
	updated = SN("updated");
	_update = SN("_update");
	state_changed = SN("state_changed");
	_get_configuration_warning = SN("_get_configuration_warning");
	changed = SN("changed");
	changed = SN("emit_changed");
	_weight_ = SN("_weight_");
	error_value = SN("error_value");
	behavior_tree = SN("behavior_tree");
	_draw_success_status = SN("_draw_success_status");
	_draw_failure_status = SN("_draw_failure_status");
	_draw_running_status = SN("_draw_running_status");
	LimboExtraClock = SN("LimboExtraClock");
	EditorIcons = SN("EditorIcons");
	BTAlwaysSucceed = SN("BTAlwaysSucceed");
	BTAlwaysFail = SN("BTAlwaysFail");
	bold = SN("bold");
	EditorFonts = SN("EditorFonts");
	item_collapsed = SN("item_collapsed");
	pressed = SN("pressed");
	StatusWarning = SN("StatusWarning");
	mode_changed = SN("mode_changed");
	connect = SN("connect");
	task_button_pressed = SN("task_button_pressed");
	gui_input = SN("gui_input");
	icon_max_width = SN("icon_max_width");
	GuiTreeArrowDown = SN("GuiTreeArrowDown");
	GuiTreeArrowRight = SN("GuiTreeArrowRight");
	font = SN("font");
	task_button_rmb = SN("task_button_rmb");
	favorite_tasks_changed = SN("favorite_tasks_changed");
	task_selected = SN("task_selected");
	toggled = SN("toggled");
	Favorites = SN("Favorites");
	Script = SN("Script");
	Help = SN("Help");
	NonFavorite = SN("NonFavorite");
	normal = SN("normal");
	LineEdit = SN("LineEdit");
	AnimationFilter = SN("AnimationFilter");
	Reload = SN("Reload");
	LimboSelectAll = SN("LimboSelectAll");
	LimboDeselectAll = SN("LimboDeselectAll");
	Search = SN("Search");
	refresh = SN("refresh");
	_draw_probability = SN("_draw_probability");
	probability_clicked = SN("probability_clicked");
	rmb_pressed = SN("rmb_pressed");
	task_activated = SN("task_activated");
	task_dragged = SN("task_dragged");
	doc_italic = SN("doc_italic");
	NodeWarning = SN("NodeWarning");
	Editor = SN("Editor");
	disabled_font_color = SN("disabled_font_color");
	font_color = SN("font_color");
	accent_color = SN("accent_color");
	font_size = SN("font_size");

	EVENT_FINISHED = "finished";
	repeat_forever.parse_utf8("Repeat ∞");
}