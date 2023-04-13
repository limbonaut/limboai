/* behavior_tree_view.h */

#ifndef BEHAVIOR_TREE_VIEW_H
#define BEHAVIOR_TREE_VIEW_H

#include "behavior_tree_data.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "scene/gui/control.h"
#include "scene/gui/tree.h"

class BehaviorTreeView : public Control {
	GDCLASS(BehaviorTreeView, Control);

private:
	Tree *tree;

public:
	BehaviorTreeView();

	void update_tree(const BehaviorTreeData &p_data);
	void clear();
};

#endif // BEHAVIOR_TREE_VIEW