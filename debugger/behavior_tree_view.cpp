/* behavior_tree_view.cpp */

#include "behavior_tree_view.h"
#include "behavior_tree_data.h"
#include "core/math/color.h"
#include "modules/limboai/bt/bt_task.h"
#include "modules/limboai/limbo_utility.h"

void BehaviorTreeView::update_tree(const BehaviorTreeData &p_data) {
	tree->clear();
	int recent_tick_usec = -1;
	TreeItem *parent = nullptr;
	List<Pair<TreeItem *, int>> parents;
	for (const BehaviorTreeData::TaskData &task_data : p_data.tasks) {
		// Figure out parent.
		parent = nullptr;
		if (parents.size()) {
			Pair<TreeItem *, int> &p = parents[0];
			parent = p.first;
			if (!(--p.second)) {
				// No children left, remove it.
				parents.pop_front();
			}
		}

		TreeItem *item = tree->create_item(parent);
		item->set_text(0, task_data.name);
		item->set_icon(0, LimboUtility::get_singleton()->get_task_icon(task_data.type_name));

		// First task in list is the root task and has most recent tick time.
		if (recent_tick_usec == -1) {
			recent_tick_usec = task_data.last_tick_usec;
		}
		// Item BG color depends on age of the task status.
		int timediff = recent_tick_usec - task_data.last_tick_usec;
		if (timediff <= 300000) {
			float alpha = (300000 - timediff) / 300000.0;
			if (task_data.status == BTTask::SUCCESS) {
				item->set_custom_bg_color(0, Color(0, 0.5, 0, alpha));
			} else if (task_data.status == BTTask::FAILURE) {
				item->set_custom_bg_color(0, Color(0.5, 0, 0, alpha));
			} else if (task_data.status == BTTask::RUNNING) {
				item->set_custom_bg_color(0, Color(0.5, 0.5, 0, alpha));
			}
		}

		// Add in front of parents stack if it expects children.
		if (task_data.num_children) {
			parents.push_front(Pair<TreeItem *, int>(item, task_data.num_children));
		}
	}
}

void BehaviorTreeView::clear() {
	tree->clear();
}

BehaviorTreeView::BehaviorTreeView() {
	tree = memnew(Tree);
	add_child(tree);
	tree->set_columns(1);
	tree->set_column_expand(0, true);
	tree->set_anchor(SIDE_RIGHT, ANCHOR_END);
	tree->set_anchor(SIDE_BOTTOM, ANCHOR_END);
}
