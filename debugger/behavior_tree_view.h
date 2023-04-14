/* behavior_tree_view.h */

#ifndef BEHAVIOR_TREE_VIEW_H
#define BEHAVIOR_TREE_VIEW_H

#include "behavior_tree_data.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "scene/gui/control.h"
#include "scene/gui/tree.h"
#include "scene/resources/style_box.h"

class BehaviorTreeView : public Control {
	GDCLASS(BehaviorTreeView, Control);

private:
	Tree *tree;
	StyleBoxFlat sbf_running;
	StyleBoxFlat sbf_success;
	StyleBoxFlat sbf_failure;
	Vector<int> collapsed_ids;

	void _draw_success_status(Object *p_obj, Rect2 p_rect);
	void _draw_running_status(Object *p_obj, Rect2 p_rect);
	void _draw_failure_status(Object *p_obj, Rect2 p_rect);
	void _item_collapsed(Object *p_obj);

protected:
	static void _bind_methods();

public:
	void update_tree(const BehaviorTreeData &p_data);
	void clear();

	BehaviorTreeView();
};

#endif // BEHAVIOR_TREE_VIEW