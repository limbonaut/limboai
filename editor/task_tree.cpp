/**
 * task_tree.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "task_tree.h"

#include "modules/limboai/bt/tasks/bt_comment.h"
#include "modules/limboai/util/limbo_utility.h"

#include "editor/editor_scale.h"

//**** TaskTree

TreeItem *TaskTree::_create_tree(const Ref<BTTask> &p_task, TreeItem *p_parent, int p_idx) {
	ERR_FAIL_COND_V(p_task.is_null(), nullptr);
	TreeItem *item = tree->create_item(p_parent, p_idx);
	item->set_metadata(0, p_task);
	// p_task->connect("changed"...)
	for (int i = 0; i < p_task->get_child_count(); i++) {
		_create_tree(p_task->get_child(i), item);
	}
	_update_item(item);
	return item;
}

void TaskTree::_update_item(TreeItem *p_item) {
	if (p_item == nullptr) {
		return;
	}
	Ref<BTTask> task = p_item->get_metadata(0);
	ERR_FAIL_COND_MSG(!task.is_valid(), "Invalid task reference in metadata.");
	p_item->set_text(0, task->get_task_name());
	if (task->is_class_ptr(BTComment::get_class_ptr_static())) {
		p_item->set_custom_font(0, (get_theme_font(SNAME("doc_italic"), SNAME("EditorFonts"))));
		p_item->set_custom_color(0, get_theme_color(SNAME("disabled_font_color"), SNAME("Editor")));
	} else if (task->get_custom_name().is_empty()) {
		p_item->set_custom_font(0, nullptr);
		p_item->clear_custom_color(0);
	} else {
		p_item->set_custom_font(0, (get_theme_font(SNAME("bold"), SNAME("EditorFonts"))));
		// p_item->set_custom_color(0, get_theme_color(SNAME("warning_color"), SNAME("Editor")));
	}
	String type_arg;
	if (task->get_script_instance() && !task->get_script_instance()->get_script()->get_path().is_empty()) {
		type_arg = task->get_script_instance()->get_script()->get_path();
	} else {
		type_arg = task->get_class();
	}
	p_item->set_icon(0, LimboUtility::get_singleton()->get_task_icon(type_arg));
	p_item->set_icon_max_width(0, 16 * EDSCALE);
	p_item->set_editable(0, false);

	for (int i = 0; i < p_item->get_button_count(0); i++) {
		p_item->erase_button(0, i);
	}

	PackedStringArray warnings = task->get_configuration_warnings();
	String warning_text;
	for (int j = 0; j < warnings.size(); j++) {
		if (j > 0) {
			warning_text += "\n";
		}
		warning_text += warnings[j];
	}
	if (!warning_text.is_empty()) {
		p_item->add_button(0, get_theme_icon(SNAME("NodeWarning"), SNAME("EditorIcons")), 0, false, warning_text);
	}

	// TODO: Update probabilities.
}

void TaskTree::_update_tree() {
	Ref<BTTask> sel;
	if (tree->get_selected()) {
		sel = tree->get_selected()->get_metadata(0);
	}

	tree->clear();
	if (bt.is_null()) {
		return;
	}

	if (bt->get_root_task().is_valid()) {
		_create_tree(bt->get_root_task(), nullptr);
	}

	TreeItem *item = _find_item(sel);
	if (item) {
		item->select(0);
	}
}

TreeItem *TaskTree::_find_item(const Ref<BTTask> &p_task) const {
	if (p_task.is_null()) {
		return nullptr;
	}
	TreeItem *item = tree->get_root();
	List<TreeItem *> stack;
	while (item && item->get_metadata(0) != p_task) {
		if (item->get_child_count() > 0) {
			stack.push_back(item->get_first_child());
		}
		item = item->get_next();
		if (item == nullptr && !stack.is_empty()) {
			item = stack.front()->get();
			stack.pop_front();
		}
	}
	return item;
}

void TaskTree::_on_item_mouse_selected(const Vector2 &p_pos, int p_button_index) {
	if (p_button_index == 2) {
		emit_signal(SNAME("rmb_pressed"), get_screen_position() + p_pos);
	}
}

void TaskTree::_on_item_selected() {
	Callable on_task_changed = callable_mp(this, &TaskTree::_on_task_changed);
	if (last_selected.is_valid()) {
		update_task(last_selected);
		if (last_selected->is_connected("changed", on_task_changed)) {
			last_selected->disconnect("changed", on_task_changed);
		}
	}
	last_selected = get_selected();
	last_selected->connect("changed", on_task_changed);
	emit_signal(SNAME("task_selected"), last_selected);
}

void TaskTree::_on_item_double_clicked() {
	emit_signal(SNAME("task_double_clicked"));
}

void TaskTree::_on_task_changed() {
	_update_item(tree->get_selected());
}

void TaskTree::load_bt(const Ref<BehaviorTree> &p_behavior_tree) {
	ERR_FAIL_COND_MSG(p_behavior_tree.is_null(), "Tried to load a null tree.");

	Callable on_task_changed = callable_mp(this, &TaskTree::_on_task_changed);
	if (last_selected.is_valid() && last_selected->is_connected("changed", on_task_changed)) {
		last_selected->disconnect("changed", on_task_changed);
	}

	bt = p_behavior_tree;
	tree->clear();
	if (bt->get_root_task().is_valid()) {
		_create_tree(bt->get_root_task(), nullptr);
	}
}

void TaskTree::unload() {
	Callable on_task_changed = callable_mp(this, &TaskTree::_on_task_changed);
	if (last_selected.is_valid() && last_selected->is_connected("changed", on_task_changed)) {
		last_selected->disconnect("changed", on_task_changed);
	}

	bt->unreference();
	tree->clear();
}

void TaskTree::update_task(const Ref<BTTask> &p_task) {
	ERR_FAIL_COND(p_task.is_null());
	TreeItem *item = _find_item(p_task);
	if (item) {
		_update_item(item);
	}
}

Ref<BTTask> TaskTree::get_selected() const {
	if (tree->get_selected()) {
		return tree->get_selected()->get_metadata(0);
	}
	return nullptr;
}

void TaskTree::deselect() {
	TreeItem *sel = tree->get_selected();
	if (sel) {
		sel->deselect(0);
	}
}

Variant TaskTree::_get_drag_data_fw(const Point2 &p_point) {
	if (editable && tree->get_item_at_position(p_point)) {
		Dictionary drag_data;
		drag_data["type"] = "task";
		drag_data["task"] = tree->get_item_at_position(p_point)->get_metadata(0);
		tree->set_drop_mode_flags(Tree::DROP_MODE_INBETWEEN | Tree::DROP_MODE_ON_ITEM);
		return drag_data;
	}
	return Variant();
}

bool TaskTree::_can_drop_data_fw(const Point2 &p_point, const Variant &p_data) const {
	if (!editable) {
		return false;
	}

	Dictionary d = p_data;
	if (!d.has("type") || !d.has("task")) {
		return false;
	}

	int section = tree->get_drop_section_at_position(p_point);
	TreeItem *item = tree->get_item_at_position(p_point);
	if (!item || section < -1) {
		return false;
	}

	if (!item->get_parent() && section != 0) { // before/after root item
		return false;
	}

	if (String(d["type"]) == "task") {
		Ref<BTTask> task = d["task"];
		const Ref<BTTask> to_task = item->get_metadata(0);
		if (task != to_task && !to_task->is_descendant_of(task)) {
			return true;
		}
	}

	return false;
}

void TaskTree::_drop_data_fw(const Point2 &p_point, const Variant &p_data) {
	Dictionary d = p_data;
	TreeItem *item = tree->get_item_at_position(p_point);
	if (item && d.has("task")) {
		Ref<BTTask> task = d["task"];
		emit_signal(SNAME("task_dragged"), task, item->get_metadata(0), tree->get_drop_section_at_position(p_point));
	}
}

void TaskTree::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED: {
			_update_tree();
		} break;
	}
}

void TaskTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_bt", "p_behavior_tree"), &TaskTree::load_bt);
	ClassDB::bind_method(D_METHOD("get_bt"), &TaskTree::get_bt);
	ClassDB::bind_method(D_METHOD("update_tree"), &TaskTree::update_tree);
	ClassDB::bind_method(D_METHOD("update_task", "p_task"), &TaskTree::update_task);
	ClassDB::bind_method(D_METHOD("get_selected"), &TaskTree::get_selected);
	ClassDB::bind_method(D_METHOD("deselect"), &TaskTree::deselect);

	ClassDB::bind_method(D_METHOD("_get_drag_data_fw"), &TaskTree::_get_drag_data_fw);
	ClassDB::bind_method(D_METHOD("_can_drop_data_fw"), &TaskTree::_can_drop_data_fw);
	ClassDB::bind_method(D_METHOD("_drop_data_fw"), &TaskTree::_drop_data_fw);

	ADD_SIGNAL(MethodInfo("rmb_pressed"));
	ADD_SIGNAL(MethodInfo("task_selected"));
	ADD_SIGNAL(MethodInfo("task_double_clicked"));
	ADD_SIGNAL(MethodInfo("task_dragged",
			PropertyInfo(Variant::OBJECT, "p_task", PROPERTY_HINT_RESOURCE_TYPE, "BTTask"),
			PropertyInfo(Variant::OBJECT, "p_to_task", PROPERTY_HINT_RESOURCE_TYPE, "BTTask"),
			PropertyInfo(Variant::INT, "p_type")));
}

TaskTree::TaskTree() {
	editable = true;

	tree = memnew(Tree);
	add_child(tree);
	tree->set_columns(2);
	tree->set_column_expand(0, true);
	tree->set_column_expand(1, false);
	tree->set_column_custom_minimum_width(1, 64);
	tree->set_anchor(SIDE_RIGHT, ANCHOR_END);
	tree->set_anchor(SIDE_BOTTOM, ANCHOR_END);
	tree->set_allow_rmb_select(true);
	tree->connect("item_mouse_selected", callable_mp(this, &TaskTree::_on_item_mouse_selected));
	tree->connect("item_selected", callable_mp(this, &TaskTree::_on_item_selected));
	tree->connect("item_activated", callable_mp(this, &TaskTree::_on_item_double_clicked));

	tree->set_drag_forwarding(callable_mp(this, &TaskTree::_get_drag_data_fw), callable_mp(this, &TaskTree::_can_drop_data_fw), callable_mp(this, &TaskTree::_drop_data_fw));
}

TaskTree::~TaskTree() {
	Callable on_task_changed = callable_mp(this, &TaskTree::_on_task_changed);
	if (last_selected.is_valid() && last_selected->is_connected("changed", on_task_changed)) {
		last_selected->disconnect("changed", on_task_changed);
	}
}
