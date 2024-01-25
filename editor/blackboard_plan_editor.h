/**
 * blackboard_plan_editor.h
 * =============================================================================
 * Copyright 2021-2024 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BLACKBOARD_PLAN_EDITOR_H
#define BLACKBOARD_PLAN_EDITOR_H

#include "../blackboard/blackboard_plan.h"

#ifdef LIMBOAI_MODULE
#include "editor/editor_inspector.h"
#include "scene/gui/dialogs.h"
#endif // LIMBOAI_MODULE

// *****

class BlackboardPlanEditor : public AcceptDialog {
	GDCLASS(BlackboardPlanEditor, AcceptDialog);

private:
	struct ThemeCache {
		Ref<Texture2D> trash_icon;
		Ref<Texture2D> grab_icon;
		Ref<StyleBoxFlat> odd_style;
		Ref<StyleBoxFlat> even_style;
		Ref<StyleBoxFlat> header_style;
	} theme_cache;

	int last_index = 0;
	int drag_mouse_y_delta = 0;
	int drag_index = -1;

	Ref<BlackboardPlan> plan;

	VBoxContainer *rows_vbox;
	Button *add_var_tool;
	PanelContainer *header_row;
	ScrollContainer *scroll_container;
	PopupMenu *type_menu;
	PopupMenu *hint_menu;

	void _add_var();
	void _trash_var(int p_index);
	void _rename_var(const String &p_new_name, int p_index);
	void _change_var_type(Variant::Type p_new_type, int p_index);
	void _change_var_hint(PropertyHint p_new_hint, int p_index);
	void _change_var_hint_string(const String &p_new_hint_string, int p_index);

	void _show_button_popup(Button *p_button, PopupMenu *p_popup, int p_index);
	void _type_chosen(int id);
	void _hint_chosen(int id);

	void _drag_button_down(Control *p_row);
	void _drag_button_up();
	void _drag_button_gui_input(const Ref<InputEvent> &p_event);

	void _refresh();
	void _visibility_changed();

protected:
	void _notification(int p_what);

public:
	void edit_plan(const Ref<BlackboardPlan> &p_plan);

	BlackboardPlanEditor();
};

// *****

class EditorInspectorPluginBBPlan : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginBBPlan, EditorInspectorPlugin);

private:
	BlackboardPlanEditor *plan_editor;
	Ref<StyleBoxFlat> toolbar_style;

	void _edit_plan(const Ref<BlackboardPlan> &p_plan);
	void _open_base_plan(const Ref<BlackboardPlan> &p_plan);

public:
	virtual bool can_handle(Object *p_object) override;
	virtual void parse_begin(Object *p_object) override;

	EditorInspectorPluginBBPlan();
};

#endif // BLACKBOARD_PLAN_EDITOR_H
