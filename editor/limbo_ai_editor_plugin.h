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
#include "task_palette.h"
#include "task_tree.h"

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
	TaskPalette *task_palette;
	HBoxContainer *fav_tasks_hbox;

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
	void _remove_task_from_favorite(const String &p_task);

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