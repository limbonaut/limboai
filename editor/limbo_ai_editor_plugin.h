/**
 * limbo_ai_editor_plugin.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED
#ifndef LIMBO_AI_EDITOR_PLUGIN_H
#define LIMBO_AI_EDITOR_PLUGIN_H

#include "modules/limboai/bt/behavior_tree.h"
#include "modules/limboai/bt/tasks/bt_task.h"

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/templates/hash_set.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tree.h"
#include "scene/resources/texture.h"

class TaskTree : public Control {
	GDCLASS(TaskTree, Control);

private:
	Tree *tree;
	Ref<BehaviorTree> bt;
	Ref<BTTask> last_selected;
	bool editable;

	TreeItem *_create_tree(const Ref<BTTask> &p_task, TreeItem *p_parent, int p_idx = -1);
	void _update_item(TreeItem *p_item);
	void _update_tree();
	TreeItem *_find_item(const Ref<BTTask> &p_task) const;

	void _on_item_selected();
	void _on_item_double_clicked();
	void _on_item_mouse_selected(const Vector2 &p_pos, int p_button_index);
	void _on_task_changed();

	Variant _get_drag_data_fw(const Point2 &p_point);
	bool _can_drop_data_fw(const Point2 &p_point, const Variant &p_data) const;
	void _drop_data_fw(const Point2 &p_point, const Variant &p_data);

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	void load_bt(const Ref<BehaviorTree> &p_behavior_tree);
	void unload();
	Ref<BehaviorTree> get_bt() const { return bt; }
	void update_tree() { _update_tree(); }
	void update_task(const Ref<BTTask> &p_task);
	Ref<BTTask> get_selected() const;
	void deselect();

	virtual bool editor_can_reload_from_file() { return false; }

	TaskTree();
	~TaskTree();
};

class TaskSection : public VBoxContainer {
	GDCLASS(TaskSection, VBoxContainer);

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
	void add_task_button(String p_name, const Ref<Texture> &icon, Variant p_meta);

	void set_collapsed(bool p_collapsed);
	bool is_collapsed() const;

	String get_category_name() const { return section_header->get_text(); }

	TaskSection(String p_category_name);
	~TaskSection();
};

class TaskPanel : public PanelContainer {
	GDCLASS(TaskPanel, PanelContainer)

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

	void _populate_core_tasks_from_class(const StringName &p_base_class, List<String> *p_task_classes);
	void _populate_from_user_dir(String p_path, HashMap<String, List<String>> *p_categories);
	void _populate_scripted_tasks_from_dir(String p_path, List<String> *p_task_classes);
	void _menu_action_selected(int p_id);
	void _on_task_button_pressed(const String &p_task);
	void _on_task_button_rmb(const String &p_task);
	void _apply_filter(const String &p_text);

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	void refresh();

	TaskPanel();
	~TaskPanel();
};

class LimboAIEditor : public Control {
	GDCLASS(LimboAIEditor, Control);

private:
	enum Action {
		ACTION_RENAME,
		ACTION_EDIT_SCRIPT,
		ACTION_OPEN_DOC,
		ACTION_MOVE_UP,
		ACTION_MOVE_DOWN,
		ACTION_DUPLICATE,
		ACTION_MAKE_ROOT,
		ACTION_REMOVE,
	};

	enum MiscMenu {
		MISC_OPEN_DEBUGGER,
		MISC_PROJECT_SETTINGS,
		MISC_CREATE_SCRIPT_TEMPLATE,
	};

	Vector<Ref<BehaviorTree>> history;
	int idx_history;
	HashSet<Ref<BehaviorTree>> dirty;

	VBoxContainer *vbox;
	Button *header;
	HSplitContainer *hsc;
	TaskTree *task_tree;
	VBoxContainer *banners;
	Panel *usage_hint;
	PopupMenu *menu;
	FileDialog *save_dialog;
	FileDialog *load_dialog;
	Button *history_back;
	Button *history_forward;
	TaskPanel *task_panel;
	HBoxContainer *fav_tasks_hbox;

	Button *comment_btn;
	Button *new_btn;
	Button *load_btn;
	Button *save_btn;
	Button *new_script_btn;
	MenuButton *misc_btn;

	ConfirmationDialog *rename_dialog;
	LineEdit *rename_edit;

	ConfirmationDialog *disk_changed;
	Tree *disk_changed_list;
	HashSet<String> disk_changed_files;

	void _add_task(const Ref<BTTask> &p_task);
	void _add_task_by_class_or_path(String p_class_or_path);
	void _remove_task(const Ref<BTTask> &p_task);
	_FORCE_INLINE_ void _add_task_with_prototype(const Ref<BTTask> &p_prototype) { _add_task(p_prototype->clone()); }
	void _update_header() const;
	void _update_history_buttons();
	void _update_favorite_tasks();
	void _update_misc_menu();
	void _update_banners();
	void _new_bt();
	void _save_bt(String p_path);
	void _load_bt(String p_path);
	void _mark_as_dirty(bool p_dirty);
	void _create_user_task_dir();
	void _edit_project_settings();

	void _reload_modified();
	void _resave_modified(String _str = "");

	void _rename_task_confirmed();

	void _on_tree_rmb(const Vector2 &p_menu_pos);
	void _action_selected(int p_id);
	void _misc_option_selected(int p_id);
	void _on_tree_task_selected(const Ref<BTTask> &p_task);
	void _on_tree_task_double_clicked();
	void _on_visibility_changed();
	void _on_header_pressed();
	void _on_save_pressed();
	void _on_history_back();
	void _on_history_forward();
	void _on_task_dragged(Ref<BTTask> p_task, Ref<BTTask> p_to_task, int p_type);
	void _on_resources_reload(const Vector<String> &p_resources);

	virtual void shortcut_input(const Ref<InputEvent> &p_event) override;

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	void edit_bt(Ref<BehaviorTree> p_behavior_tree, bool p_force_refresh = false);

	void apply_changes();

	LimboAIEditor();
	~LimboAIEditor();
};

class LimboAIEditorPlugin : public EditorPlugin {
	GDCLASS(LimboAIEditorPlugin, EditorPlugin);

private:
	LimboAIEditor *limbo_ai_editor;

protected:
	void _notification(int p_notification);

public:
	virtual String get_name() const override { return "LimboAI"; }
	virtual const Ref<Texture2D> get_icon() const override;
	bool has_main_screen() const override { return true; }
	virtual void make_visible(bool p_visible) override;
	virtual void apply_changes() override;
	virtual void edit(Object *p_object) override;
	virtual bool handles(Object *p_object) const override;

	LimboAIEditorPlugin();
	~LimboAIEditorPlugin();
};

#endif // LIMBO_AI_EDITOR_PLUGIN_H

#endif // TOOLS_ENABLED