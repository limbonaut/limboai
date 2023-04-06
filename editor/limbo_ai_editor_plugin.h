/* limbo_ai_editor_plugin.h */

#ifdef TOOLS_ENABLED
#ifndef LIMBO_AI_EDITOR_PLUGIN_H
#define LIMBO_AI_EDITOR_PLUGIN_H

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/templates/hash_set.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "modules/limboai/bt/behavior_tree.h"
#include "modules/limboai/bt/bt_task.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/line_edit.h"
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

	void _on_task_button_pressed(const StringName &p_task);
	void _on_header_pressed();

protected:
	static void _bind_methods();

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
	LineEdit *filter_edit;
	VBoxContainer *sections;

	void _populate_core_tasks_from_class(const StringName &p_base_class, List<String> *p_task_classes);
	void _populate_from_user_dir(String p_path, HashMap<String, List<String>> *p_categories);
	void _populate_scripted_tasks_from_dir(String p_path, List<String> *p_task_classes);
	void _on_task_button_pressed(const StringName &p_task);
	void _on_filter_text_changed(String p_text);

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
		ACTION_REMOVE,
		ACTION_MOVE_UP,
		ACTION_MOVE_DOWN,
		ACTION_DUPLICATE,
		ACTION_MAKE_ROOT,
	};

	Vector<Ref<BehaviorTree>> history;
	int idx_history;
	HashSet<Ref<BehaviorTree>> dirty;

	Button *header;
	HSplitContainer *hsc;
	TaskTree *task_tree;
	Panel *usage_hint;
	PopupMenu *menu;
	FileDialog *save_dialog;
	FileDialog *load_dialog;
	Button *history_back;
	Button *history_forward;
	TaskPanel *task_panel;

	ConfirmationDialog *rename_dialog;
	LineEdit *rename_edit;

	ConfirmationDialog *disk_changed;
	Tree *disk_changed_list;
	HashSet<String> disk_changed_files;

	void _add_task(const Ref<BTTask> &p_task);
	void _remove_task(const Ref<BTTask> &p_task);
	_FORCE_INLINE_ void _add_task_with_prototype(const Ref<BTTask> &p_prototype) { _add_task(p_prototype->clone()); }
	void _update_header() const;
	void _update_history_buttons();
	void _new_bt();
	void _save_bt(String p_path);
	void _load_bt(String p_path);
	void _mark_as_dirty(bool p_dirty);

	void _reload_modified();
	void _resave_modified(String _str = "");

	void _rename_task_confirmed();

	void _on_tree_rmb(const Vector2 &p_menu_pos);
	void _on_action_selected(int p_id);
	void _on_tree_task_selected(const Ref<BTTask> &p_task);
	void _on_tree_task_double_clicked();
	void _on_visibility_changed();
	void _on_header_pressed();
	void _on_save_pressed();
	void _on_panel_task_selected(String p_task);
	void _on_history_back();
	void _on_history_forward();
	void _on_task_dragged(Ref<BTTask> p_task, Ref<BTTask> p_to_task, int p_type);
	void _on_resources_reload(const Vector<String> &p_resources);

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	static Ref<Texture> get_task_icon(String p_script_path_or_class);

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

#endif // TOOLS_ENABLED

#endif // LIMBO_AI_EDITOR_PLUGIN_H