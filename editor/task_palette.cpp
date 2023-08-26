/**
 * task_palette.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "task_palette.h"

#include "modules/limboai/util/limbo_task_db.h"
#include "modules/limboai/util/limbo_utility.h"

#include "core/config/project_settings.h"
#include "editor/editor_help.h"
#include "editor/editor_node.h"
#include "editor/editor_paths.h"
#include "editor/editor_scale.h"
#include "editor/plugins/script_editor_plugin.h"

//**** TaskButton

Control *TaskButton::make_custom_tooltip(const String &p_text) const {
	EditorHelpBit *help_bit = memnew(EditorHelpBit);
	help_bit->get_rich_text()->set_custom_minimum_size(Size2(360 * EDSCALE, 1));

	String help_text;
	if (!p_text.is_empty()) {
		help_text = p_text;
	} else {
		help_text = "[i]" + TTR("No description.") + "[/i]";
	}

	help_bit->set_text(help_text);

	return help_bit;
}

//**** TaskButton ^

//**** TaskPaletteSection

void TaskPaletteSection::_on_task_button_pressed(const String &p_task) {
	emit_signal(SNAME("task_button_pressed"), p_task);
}

void TaskPaletteSection::_on_task_button_gui_input(const Ref<InputEvent> &p_event, const String &p_task) {
	if (!p_event->is_pressed()) {
		return;
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == MouseButton::RIGHT) {
		emit_signal(SNAME("task_button_rmb"), p_task);
	}
}

void TaskPaletteSection::_on_header_pressed() {
	set_collapsed(!is_collapsed());
}

void TaskPaletteSection::set_filter(String p_filter_text) {
	int num_hidden = 0;
	if (p_filter_text.is_empty()) {
		for (int i = 0; i < tasks_container->get_child_count(); i++) {
			Object::cast_to<Button>(tasks_container->get_child(i))->show();
		}
		set_visible(tasks_container->get_child_count() > 0);
	} else {
		for (int i = 0; i < tasks_container->get_child_count(); i++) {
			Button *btn = Object::cast_to<Button>(tasks_container->get_child(i));
			btn->set_visible(btn->get_text().findn(p_filter_text) != -1);
			num_hidden += !btn->is_visible();
		}
		set_visible(num_hidden < tasks_container->get_child_count());
	}
}

void TaskPaletteSection::add_task_button(const String &p_name, const Ref<Texture> &icon, const String &p_tooltip, Variant p_meta) {
	TaskButton *btn = memnew(TaskButton);
	btn->set_text(p_name);
	btn->set_icon(icon);
	btn->set_tooltip_text(p_tooltip);
	btn->add_theme_constant_override(SNAME("icon_max_width"), 16 * EDSCALE); // Force user icons to  be of the proper size.
	btn->connect(SNAME("pressed"), callable_mp(this, &TaskPaletteSection::_on_task_button_pressed).bind(p_meta));
	btn->connect(SNAME("gui_input"), callable_mp(this, &TaskPaletteSection::_on_task_button_gui_input).bind(p_meta));
	tasks_container->add_child(btn);
}

void TaskPaletteSection::set_collapsed(bool p_collapsed) {
	tasks_container->set_visible(!p_collapsed);
	section_header->set_icon(p_collapsed ? get_theme_icon(SNAME("GuiTreeArrowRight"), SNAME("EditorIcons")) : get_theme_icon(SNAME("GuiTreeArrowDown"), SNAME("EditorIcons")));
}

bool TaskPaletteSection::is_collapsed() const {
	return !tasks_container->is_visible();
}

void TaskPaletteSection::_notification(int p_what) {
	if (p_what == NOTIFICATION_THEME_CHANGED) {
		section_header->set_icon(is_collapsed() ? get_theme_icon(SNAME("GuiTreeArrowRight"), SNAME("EditorIcons")) : get_theme_icon(SNAME("GuiTreeArrowDown"), SNAME("EditorIcons")));
		section_header->add_theme_font_override(SNAME("font"), get_theme_font(SNAME("bold"), SNAME("EditorFonts")));
	}
}

void TaskPaletteSection::_bind_methods() {
	ADD_SIGNAL(MethodInfo("task_button_pressed"));
	ADD_SIGNAL(MethodInfo("task_button_rmb"));
}

TaskPaletteSection::TaskPaletteSection(String p_category_name) {
	section_header = memnew(Button);
	add_child(section_header);
	section_header->set_text(p_category_name);
	section_header->set_focus_mode(FOCUS_NONE);
	section_header->connect("pressed", callable_mp(this, &TaskPaletteSection::_on_header_pressed));

	tasks_container = memnew(HFlowContainer);
	add_child(tasks_container);
}

TaskPaletteSection::~TaskPaletteSection() {
}

//**** TaskPaletteSection ^

//**** TaskPalette

void TaskPalette::_menu_action_selected(int p_id) {
	ERR_FAIL_COND(context_task.is_empty());
	switch (p_id) {
		case MENU_OPEN_DOC: {
			String help_class;
			if (context_task.begins_with("res://")) {
				Ref<Script> s = ResourceLoader::load(context_task, "Script");
				help_class = s->get_language()->get_global_class_name(context_task);
			}
			if (help_class.is_empty()) {
				// Assuming context task is core class.
				help_class = context_task;
			}
			ScriptEditor::get_singleton()->goto_help("class_name:" + help_class);
			EditorNode::get_singleton()->set_visible_editor(EditorNode::EDITOR_SCRIPT);
		} break;
		case MENU_EDIT_SCRIPT: {
			ERR_FAIL_COND(!context_task.begins_with("res://"));
			ScriptEditor::get_singleton()->open_file(context_task);
		} break;
		case MENU_FAVORITE: {
			PackedStringArray favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
			if (favorite_tasks.has(context_task)) {
				favorite_tasks.erase(context_task);
			} else {
				favorite_tasks.append(context_task);
			}
			ProjectSettings::get_singleton()->set_setting("limbo_ai/behavior_tree/favorite_tasks", favorite_tasks);
			ProjectSettings::get_singleton()->save();
			emit_signal(SNAME("favorite_tasks_changed"));
		} break;
	}
}

void TaskPalette::_on_task_button_pressed(const String &p_task) {
	emit_signal(SNAME("task_selected"), p_task);
}

void TaskPalette::_on_task_button_rmb(const String &p_task) {
	ERR_FAIL_COND(p_task.is_empty());

	context_task = p_task;
	menu->clear();

	menu->add_icon_item(get_theme_icon(SNAME("Script"), SNAME("EditorIcons")), TTR("Edit Script"), MENU_EDIT_SCRIPT);
	menu->set_item_disabled(MENU_EDIT_SCRIPT, !context_task.begins_with("res://"));
	menu->add_icon_item(get_theme_icon(SNAME("Help"), SNAME("EditorIcons")), TTR("Open Documentation"), MENU_OPEN_DOC);

	menu->add_separator();
	Array favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
	if (favorite_tasks.has(context_task)) {
		menu->add_icon_item(get_theme_icon(SNAME("NonFavorite"), SNAME("EditorIcons")), TTR("Remove from Favorites"), MENU_FAVORITE);
	} else {
		menu->add_icon_item(get_theme_icon(SNAME("Favorites"), SNAME("EditorIcons")), TTR("Add to Favorites"), MENU_FAVORITE);
	}

	menu->reset_size();
	menu->set_position(get_screen_position() + get_local_mouse_position());
	menu->popup();
}

void TaskPalette::_apply_filter(const String &p_text) {
	for (int i = 0; i < sections->get_child_count(); i++) {
		TaskPaletteSection *sec = Object::cast_to<TaskPaletteSection>(sections->get_child(i));
		ERR_FAIL_COND(sec == nullptr);
		sec->set_filter(p_text);
	}
}

void TaskPalette::refresh() {
	filter_edit->set_right_icon(get_theme_icon(SNAME("Search"), SNAME("EditorIcons")));

	HashSet<String> collapsed_sections;
	if (sections->get_child_count() == 0) {
		// Restore collapsed state from config.
		ConfigFile conf;
		String conf_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("limbo_ai.cfg");
		if (conf.load(conf_path) == OK) {
			Variant value = conf.get_value("bt_editor", "collapsed_sections", Array());
			if (value.is_array()) {
				Array arr = value;
				for (int i = 0; i < arr.size(); i++) {
					if (arr[i].get_type() == Variant::STRING) {
						collapsed_sections.insert(arr[i]);
					}
				}
			}
		}
	} else {
		for (int i = 0; i < sections->get_child_count(); i++) {
			TaskPaletteSection *sec = Object::cast_to<TaskPaletteSection>(sections->get_child(i));
			if (sec->is_collapsed()) {
				collapsed_sections.insert(sec->get_category_name());
			}
			sections->get_child(i)->queue_free();
		}
	}

	LimboTaskDB::scan_user_tasks();
	List<String> categories = LimboTaskDB::get_categories();
	categories.sort();

	for (String cat : categories) {
		List<String> tasks = LimboTaskDB::get_tasks_in_category(cat);

		if (tasks.size() == 0) {
			continue;
		}

		TaskPaletteSection *sec = memnew(TaskPaletteSection(cat));
		for (String task_meta : tasks) {
			Ref<Texture2D> icon = LimboUtility::get_singleton()->get_task_icon(task_meta);

			String tname;
			DocTools *dd = EditorHelp::get_doc_data();
			HashMap<String, DocData::ClassDoc>::Iterator E;
			if (task_meta.begins_with("res:")) {
				tname = task_meta.get_file().get_basename().trim_prefix("BT").to_pascal_case();
				E = dd->class_list.find(vformat("\"%s\"", task_meta.trim_prefix("res://")));
				if (!E) {
					E = dd->class_list.find(tname);
				}
			} else {
				tname = task_meta.trim_prefix("BT");
				E = dd->class_list.find(task_meta);
			}

			String descr;
			if (E) {
				if (E->value.description.is_empty() || E->value.description.length() > 1000) {
					descr = DTR(E->value.brief_description);
				} else {
					descr = DTR(E->value.description);
				}
			}

			sec->add_task_button(tname, icon, descr, task_meta);
		}
		sec->set_filter("");
		sec->connect(SNAME("task_button_pressed"), callable_mp(this, &TaskPalette::_on_task_button_pressed));
		sec->connect(SNAME("task_button_rmb"), callable_mp(this, &TaskPalette::_on_task_button_rmb));
		sections->add_child(sec);
		sec->set_collapsed(collapsed_sections.has(cat));
	}

	if (!filter_edit->get_text().is_empty()) {
		_apply_filter(filter_edit->get_text());
	}
}

void TaskPalette::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_EXIT_TREE: {
			if (sections->get_child_count() == 0) {
				return;
			}
			Array collapsed_sections;
			for (int i = 0; i < sections->get_child_count(); i++) {
				TaskPaletteSection *sec = Object::cast_to<TaskPaletteSection>(sections->get_child(i));
				if (sec->is_collapsed()) {
					collapsed_sections.push_back(sec->get_category_name());
				}
			}
			ConfigFile conf;
			String conf_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("limbo_ai.cfg");
			conf.load(conf_path);
			conf.set_value("bt_editor", "collapsed_sections", collapsed_sections);
			conf.save(conf_path);
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			refresh_btn->set_icon(get_theme_icon(SNAME("Reload"), SNAME("EditorIcons")));
			if (is_visible_in_tree()) {
				refresh();
			}
		} break;
	}
}

void TaskPalette::_bind_methods() {
	ClassDB::bind_method(D_METHOD("refresh"), &TaskPalette::refresh);

	ADD_SIGNAL(MethodInfo("task_selected"));
	ADD_SIGNAL(MethodInfo("favorite_tasks_changed"));
}

TaskPalette::TaskPalette() {
	VBoxContainer *vb = memnew(VBoxContainer);
	add_child(vb);

	HBoxContainer *hb = memnew(HBoxContainer);
	vb->add_child(hb);

	filter_edit = memnew(LineEdit);
	filter_edit->set_clear_button_enabled(true);
	filter_edit->set_placeholder(TTR("Filter tasks"));
	filter_edit->connect("text_changed", callable_mp(this, &TaskPalette::_apply_filter));
	filter_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	hb->add_child(filter_edit);

	refresh_btn = memnew(Button);
	refresh_btn->set_tooltip_text(TTR("Refresh tasks"));
	refresh_btn->set_flat(true);
	refresh_btn->set_focus_mode(FocusMode::FOCUS_NONE);
	refresh_btn->connect("pressed", callable_mp(this, &TaskPalette::refresh));
	hb->add_child(refresh_btn);

	ScrollContainer *sc = memnew(ScrollContainer);
	sc->set_h_size_flags(SIZE_EXPAND_FILL);
	sc->set_v_size_flags(SIZE_EXPAND_FILL);
	vb->add_child(sc);

	sections = memnew(VBoxContainer);
	sections->set_h_size_flags(SIZE_EXPAND_FILL);
	sections->set_v_size_flags(SIZE_EXPAND_FILL);
	sc->add_child(sections);

	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", callable_mp(this, &TaskPalette::_menu_action_selected));
}

TaskPalette::~TaskPalette() {
}