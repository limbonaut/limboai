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
#include "scene/gui/flow_container.h"
#include "scene/gui/line_edit.h"

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

	LineEdit *filter_edit;
	VBoxContainer *sections;
	PopupMenu *menu;
	Button *refresh_btn;

	String context_task;

	void _menu_action_selected(int p_id);
	void _on_task_button_pressed(const String &p_task);
	void _on_task_button_rmb(const String &p_task);
	void _apply_filter(const String &p_text);

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	void refresh();

	TaskPalette();
	~TaskPalette();
};

#endif // TASK_PALETTE