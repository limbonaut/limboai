/**
 * task_tree.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "modules/limboai/bt/behavior_tree.h"

#include "scene/gui/control.h"
#include "scene/gui/tree.h"

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