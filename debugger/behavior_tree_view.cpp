/* behavior_tree_view.cpp */

#include "behavior_tree_view.h"
#include "behavior_tree_data.h"
#include "core/math/color.h"
#include "core/math/math_defs.h"
#include "core/object/callable_method_pointer.h"
#include "core/typedefs.h"
#include "editor/editor_scale.h"
#include "modules/limboai/bt/bt_task.h"
#include "modules/limboai/limbo_utility.h"
#include "scene/resources/style_box.h"

void BehaviorTreeView::_draw_running_status(Object *p_obj, Rect2 p_rect) {
	p_rect = p_rect.grow_side(SIDE_LEFT, p_rect.get_position().x);
	sbf_running.draw(tree->get_canvas_item(), p_rect);
}

void BehaviorTreeView::_draw_success_status(Object *p_obj, Rect2 p_rect) {
	p_rect = p_rect.grow_side(SIDE_LEFT, p_rect.get_position().x);
	sbf_success.draw(tree->get_canvas_item(), p_rect);
}

void BehaviorTreeView::_draw_failure_status(Object *p_obj, Rect2 p_rect) {
	p_rect = p_rect.grow_side(SIDE_LEFT, p_rect.get_position().x);
	sbf_failure.draw(tree->get_canvas_item(), p_rect);
}

void BehaviorTreeView::_item_collapsed(Object *p_obj) {
	TreeItem *item = Object::cast_to<TreeItem>(p_obj);
	if (!item) {
		return;
	}
	int id = item->get_metadata(0);
	bool collapsed = item->is_collapsed();
	if (!collapsed_ids.has(id) && collapsed) {
		collapsed_ids.push_back(item->get_metadata(0));
	} else if (collapsed_ids.has(id) && !collapsed) {
		collapsed_ids.erase(id);
	}
}

void BehaviorTreeView::update_tree(const BehaviorTreeData &p_data) {
	// Remember selected.
	int selected_id = -1;
	if (tree->get_selected()) {
		selected_id = tree->get_selected()->get_metadata(0);
	}

	tree->clear();
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
		// Do this first because it resets properties of the cell...
		item->set_cell_mode(0, TreeItem::CELL_MODE_CUSTOM);
		item->set_cell_mode(1, TreeItem::CELL_MODE_ICON);

		item->set_metadata(0, task_data.id);
		item->set_text(0, task_data.name);
		item->set_icon(0, LimboUtility::get_singleton()->get_task_icon(task_data.type_name));
		item->set_text(2, rtos(Math::snapped(task_data.elapsed_time, 0.01)).pad_decimals(2));

		if (task_data.status == BTTask::SUCCESS) {
			item->set_custom_draw(0, this, SNAME("_draw_success_status"));
			item->set_icon(1, icon_success);
		} else if (task_data.status == BTTask::FAILURE) {
			item->set_custom_draw(0, this, SNAME("_draw_failure_status"));
			item->set_icon(1, icon_failure);
		} else if (task_data.status == BTTask::RUNNING) {
			item->set_custom_draw(0, this, SNAME("_draw_running_status"));
			item->set_icon(1, icon_running);
		}

		if (task_data.id == selected_id) {
			tree->set_selected(item);
		}

		if (collapsed_ids.has(task_data.id)) {
			item->set_collapsed(true);
		}

		// Add in front of parents stack if it expects children.
		if (task_data.num_children) {
			parents.push_front(Pair<TreeItem *, int>(item, task_data.num_children));
		}
	}
}

void BehaviorTreeView::clear() {
	tree->clear();
	collapsed_ids.clear();
}

void BehaviorTreeView::_notification(int p_notification) {
	if (p_notification == NOTIFICATION_THEME_CHANGED) {
		icon_running = get_theme_icon(SNAME("LimboExtraClock"), SNAME("EditorIcons"));
		icon_success = get_theme_icon(SNAME("BTAlwaysSucceed"), SNAME("EditorIcons"));
		icon_failure = get_theme_icon(SNAME("BTAlwaysFail"), SNAME("EditorIcons"));
	}
}

void BehaviorTreeView::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_draw_running_status"), &BehaviorTreeView::_draw_running_status);
	ClassDB::bind_method(D_METHOD("_draw_success_status"), &BehaviorTreeView::_draw_success_status);
	ClassDB::bind_method(D_METHOD("_draw_failure_status"), &BehaviorTreeView::_draw_failure_status);
	ClassDB::bind_method(D_METHOD("_item_collapsed"), &BehaviorTreeView::_item_collapsed);
}

BehaviorTreeView::BehaviorTreeView() {
	tree = memnew(Tree);
	add_child(tree);
	tree->set_columns(3);
	tree->set_column_expand(0, true);
	tree->set_column_expand(1, false);
	tree->set_column_expand(2, false);
	tree->set_column_custom_minimum_width(1, 18.0 * EDSCALE);
	tree->set_column_custom_minimum_width(2, 40.0 * EDSCALE);
	tree->set_anchor(SIDE_RIGHT, ANCHOR_END);
	tree->set_anchor(SIDE_BOTTOM, ANCHOR_END);

	sbf_running.set_border_color(Color(1.0, 1.0, 0.0));
	sbf_running.set_bg_color(Color(1.0, 1.0, 0, 0.1));
	sbf_running.set_border_width(SIDE_LEFT, 4.0);
	sbf_running.set_border_width(SIDE_RIGHT, 4.0);

	sbf_success.set_border_color(Color(0.0, 0.8, 0.0));
	sbf_success.set_bg_color(Color(0.0, 0.8, 0.0, 0.1));
	sbf_success.set_border_width(SIDE_LEFT, 4.0);
	sbf_success.set_border_width(SIDE_RIGHT, 4.0);

	sbf_failure.set_border_color(Color(1.0, 0.0, 0.0));
	sbf_failure.set_bg_color(Color(1.0, 0.0, 0.0, 0.1));
	sbf_failure.set_border_width(SIDE_LEFT, 4.0);
	sbf_failure.set_border_width(SIDE_RIGHT, 4.0);

	tree->connect(SNAME("item_collapsed"), callable_mp(this, &BehaviorTreeView::_item_collapsed));
}
