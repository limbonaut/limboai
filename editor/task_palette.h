/**
 * task_palette.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#ifndef TASK_PALETTE_H
#define TASK_PALETTE_H

#include "scene/gui/panel_container.h"

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/check_box.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/popup.h"

class TaskButton : public Button {
	GDCLASS(TaskButton, Button);

public:
	virtual Control *make_custom_tooltip(const String &p_text) const override;
};

class TaskPaletteSection : public VBoxContainer {
	GDCLASS(TaskPaletteSection, VBoxContainer);

private:
	FlowContainer *tasks_container;
	Button *section_header;

	void _on_task_button_pressed(const String &p_task);
	void _on_task_button_gui_input(const Ref<InputEvent> &p_event, const String &p_task);
	void _on_header_pressed();

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	void set_filter(String p_filter);
	void add_task_button(const String &p_name, const Ref<Texture> &icon, const String &p_tooltip, Variant p_meta);

	void set_collapsed(bool p_collapsed);
	bool is_collapsed() const;

	String get_category_name() const { return section_header->get_text(); }

	TaskPaletteSection(String p_category_name);
	~TaskPaletteSection();
};

class TaskPalette : public PanelContainer {
	GDCLASS(TaskPalette, PanelContainer)

private:
	enum MenuAction {
		MENU_EDIT_SCRIPT,
		MENU_OPEN_DOC,
		MENU_FAVORITE,
	};

	struct FilterSettings {
		enum TypeFilter {
			TYPE_ALL,
			TYPE_CORE,
			TYPE_USER,
		} type_filter;

		enum CategoryFilter {
			CATEGORY_ALL,
			CATEGORY_INCLUDE,
			CATEGORY_EXCLUDE,
		} category_filter;

		HashSet<String> excluded_categories;
	} filter_settings;

	LineEdit *filter_edit;
	VBoxContainer *sections;
	PopupMenu *menu;
	Button *tool_filters;
	Button *tool_refresh;

	// Filter popup
	PopupPanel *filter_popup;
	Button *type_all;
	Button *type_core;
	Button *type_user;
	Button *category_all;
	Button *category_include;
	Button *category_exclude;
	VBoxContainer *category_choice;
	Button *select_all;
	Button *deselect_all;
	ScrollContainer *category_scroll;
	VBoxContainer *category_list;
	Ref<StyleBox> category_choice_background;

	String context_task;

	void _menu_action_selected(int p_id);
	void _on_task_button_pressed(const String &p_task);
	void _on_task_button_rmb(const String &p_task);
	void _apply_filter(const String &p_text);
	void _update_filter_popup();
	void _show_filter_popup();
	void _type_filter_changed();
	void _category_filter_changed();
	void _set_all_filter_categories(bool p_selected);
	void _category_item_toggled(bool p_pressed, const String &p_category);
	void _filter_data_changed();
	void _draw_filter_popup_background();

	_FORCE_INLINE_ void _set_category_excluded(const String &p_category, bool p_excluded) {
		if (p_excluded) {
			filter_settings.excluded_categories.insert(p_category);
		} else {
			filter_settings.excluded_categories.erase(p_category);
		}
	}

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	void refresh();

	TaskPalette();
	~TaskPalette();
};

#endif // TASK_PALETTE