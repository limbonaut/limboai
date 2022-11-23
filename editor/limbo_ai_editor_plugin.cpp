/* limbo_ai_editor_plugin.cpp */

#ifdef TOOLS_ENABLED

#include "limbo_ai_editor_plugin.h"

#include "core/array.h"
#include "core/class_db.h"
#include "core/dictionary.h"
#include "core/error_list.h"
#include "core/error_macros.h"
#include "core/io/config_file.h"
#include "core/io/image_loader.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/list.h"
#include "core/math/math_defs.h"
#include "core/object.h"
#include "core/os/dir_access.h"
#include "core/os/memory.h"
#include "core/print_string.h"
#include "core/project_settings.h"
#include "core/resource.h"
#include "core/script_language.h"
#include "core/string_name.h"
#include "core/typedefs.h"
#include "core/ustring.h"
#include "core/variant.h"
#include "core/vector.h"
#include "editor/editor_inspector.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"
#include "modules/limboai/bt/actions/bt_action.h"
#include "modules/limboai/bt/behavior_tree.h"
#include "modules/limboai/bt/bt_task.h"
#include "modules/limboai/bt/composites/bt_parallel.h"
#include "modules/limboai/bt/composites/bt_selector.h"
#include "modules/limboai/bt/composites/bt_sequence.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/control.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/separator.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tree.h"
#include <cstddef>

//////////////////////////////  TaskTree  //////////////////////////////////////

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
	ERR_FAIL_COND_MSG(p_item == nullptr, "Argument \"p_item\" is null.");
	Ref<BTTask> task = p_item->get_metadata(0);
	ERR_FAIL_COND_MSG(!task.is_valid(), "Invalid task reference in metadata.");
	p_item->set_text(0, task->get_task_name());
	if (task->get_script_instance() && !task->get_script_instance()->get_script()->get_path().empty()) {
		p_item->set_icon(0, LimboAIEditor::get_task_icon(task->get_script_instance()->get_script()->get_path()));
	} else {
		p_item->set_icon(0, LimboAIEditor::get_task_icon(task->get_class()));
	}
	p_item->set_editable(0, false);

	for (int i = 0; i < p_item->get_button_count(0); i++) {
		p_item->erase_button(0, i);
	}
	String warning = task->get_configuration_warning();
	if (!warning.empty()) {
		p_item->add_button(0, get_icon("NodeWarning", "EditorIcons"), 0, false, warning);
	}

	// TODO: Update probabilities.
}

void TaskTree::_update_tree() {
	Ref<BTTask> sel;
	if (tree->get_selected()) {
		sel = tree->get_selected()->get_metadata(0);
	}

	tree->clear();
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
		if (item->get_children()) {
			stack.push_back(item->get_children());
		}
		item = item->get_next();
		if (item == nullptr && !stack.empty()) {
			item = stack.front()->get();
			stack.pop_front();
		}
	}
	return item;
}

void TaskTree::_on_item_rmb_selected(const Vector2 &p_pos) {
	emit_signal("rmb_pressed", tree->get_global_transform().xform(p_pos));
}

void TaskTree::_on_item_selected() {
	if (last_selected.is_valid()) {
		update_task(last_selected);
		if (last_selected->is_connected("changed", this, "_on_task_changed")) {
			last_selected->disconnect("changed", this, "_on_task_changed");
		}
	}
	last_selected = get_selected();
	last_selected->connect("changed", this, "_on_task_changed");
	emit_signal("task_selected", last_selected);
}

void TaskTree::_on_task_changed() {
	_update_item(tree->get_selected());
}

void TaskTree::load_bt(const Ref<BehaviorTree> &p_behavior_tree) {
	ERR_FAIL_COND_MSG(p_behavior_tree.is_null(), "Tried to load a null tree.");

	if (last_selected.is_valid() and last_selected->is_connected("changed", this, "_on_task_changed")) {
		last_selected->disconnect("changed", this, "_on_task_changed");
	}

	bt = p_behavior_tree;
	tree->clear();
	if (bt->get_root_task().is_valid()) {
		_create_tree(bt->get_root_task(), nullptr);
	}
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

Variant TaskTree::get_drag_data_fw(const Point2 &p_point, Control *p_from) {
	if (editable && tree->get_item_at_position(p_point)) {
		Dictionary drag_data;
		drag_data["type"] = "task";
		drag_data["task"] = tree->get_item_at_position(p_point)->get_metadata(0);
		tree->set_drop_mode_flags(Tree::DROP_MODE_INBETWEEN | Tree::DROP_MODE_ON_ITEM);
		return drag_data;
	}
	return Variant();
}

bool TaskTree::can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const {
	if (!editable) {
		return false;
	}

	Dictionary d = p_data;
	if (!d.has("type") || !d.has("task")) {
		return false;
	}

	int section = tree->get_drop_section_at_position(p_point);
	TreeItem *item = tree->get_item_at_position(p_point);
	if (!item || section < -1 || (section == -1 && !item->get_parent())) {
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

void TaskTree::drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) {
	Dictionary d = p_data;
	TreeItem *item = tree->get_item_at_position(p_point);
	if (item && d.has("task")) {
		Ref<BTTask> task = d["task"];
		emit_signal("task_dragged", task, item->get_metadata(0), tree->get_drop_section_at_position(p_point));
	}
}

void TaskTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_item_rmb_selected"), &TaskTree::_on_item_rmb_selected);
	ClassDB::bind_method(D_METHOD("_on_item_selected"), &TaskTree::_on_item_selected);
	ClassDB::bind_method(D_METHOD("_on_task_changed"), &TaskTree::_on_task_changed);
	ClassDB::bind_method(D_METHOD("load_bt", "p_behavior_tree"), &TaskTree::load_bt);
	ClassDB::bind_method(D_METHOD("get_bt"), &TaskTree::get_bt);
	ClassDB::bind_method(D_METHOD("update_tree"), &TaskTree::update_tree);
	ClassDB::bind_method(D_METHOD("update_task", "p_task"), &TaskTree::update_task);
	ClassDB::bind_method(D_METHOD("get_selected"), &TaskTree::get_selected);
	ClassDB::bind_method(D_METHOD("deselect"), &TaskTree::deselect);

	ClassDB::bind_method(D_METHOD("get_drag_data_fw"), &TaskTree::get_drag_data_fw);
	ClassDB::bind_method(D_METHOD("can_drop_data_fw"), &TaskTree::can_drop_data_fw);
	ClassDB::bind_method(D_METHOD("drop_data_fw"), &TaskTree::drop_data_fw);

	ADD_SIGNAL(MethodInfo("rmb_pressed"));
	ADD_SIGNAL(MethodInfo("task_selected"));
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
	tree->set_column_min_width(1, 64);
	tree->set_anchor(MARGIN_RIGHT, ANCHOR_END);
	tree->set_anchor(MARGIN_BOTTOM, ANCHOR_END);
	tree->set_allow_rmb_select(true);
	tree->connect("item_rmb_selected", this, "_on_item_rmb_selected");
	tree->connect("item_selected", this, "_on_item_selected");
	tree->set_drag_forwarding(this);
}

TaskTree::~TaskTree() {
	if (last_selected.is_valid() and last_selected->is_connected("changed", this, "_on_task_changed")) {
		last_selected->disconnect("changed", this, "_on_task_changed");
	}
}

//////////////////////////////  TaskTree  //////////////////////////////////////

////////////////////////////// TaskSection  ////////////////////////////////////

void TaskSection::_on_task_button_pressed(const StringName &p_task) {
	emit_signal("task_button_pressed", p_task);
}

void TaskSection::_on_header_pressed() {
	tasks_container->set_visible(!tasks_container->is_visible());
	section_header->set_icon(tasks_container->is_visible() ? get_icon("GuiTreeArrowDown", "EditorIcons") : get_icon("GuiTreeArrowRight", "EditorIcons"));
}

void TaskSection::set_filter(String p_filter_text) {
	int num_hidden = 0;
	if (p_filter_text.empty()) {
		for (int i = 0; i < tasks_container->get_child_count(); i++) {
			Object::cast_to<Button>(tasks_container->get_child(i))->show();
		}
		set_visible(tasks_container->get_child_count() > 0);
	} else {
		for (int i = 0; i < tasks_container->get_child_count(); i++) {
			Button *btn = Object::cast_to<Button>(tasks_container->get_child(i));
			btn->set_visible(btn->get_text().findn(p_filter_text) != -1);
			num_hidden += !btn->is_visible();
		}
		set_visible(num_hidden < tasks_container->get_child_count());
	}
}

void TaskSection::add_task_button(String p_name, const Ref<Texture> &icon, Variant p_meta) {
	Button *btn = memnew(Button);
	btn->set_text(p_name);
	btn->set_icon(icon);
	btn->connect("pressed", this, "_on_task_button_pressed", varray(p_meta));
	tasks_container->add_child(btn);
}

void TaskSection::set_collapsed(bool p_collapsed) {
	tasks_container->set_visible(!p_collapsed);
	section_header->set_icon(p_collapsed ? get_icon("GuiTreeArrowRight", "EditorIcons") : get_icon("GuiTreeArrowDown", "EditorIcons"));
}

bool TaskSection::is_collapsed() const {
	return !tasks_container->is_visible();
}

void TaskSection::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_task_button_pressed", "p_class"), &TaskSection::_on_task_button_pressed);
	ClassDB::bind_method(D_METHOD("_on_header_pressed"), &TaskSection::_on_header_pressed);

	ADD_SIGNAL(MethodInfo("task_button_pressed"));
}

TaskSection::TaskSection(String p_category_name, EditorNode *p_editor) {
	section_header = memnew(Button);
	add_child(section_header);
	section_header->set_text(p_category_name);
	section_header->set_icon(p_editor->get_gui_base()->get_icon("GuiTreeArrowDown", "EditorIcons"));
	section_header->set_focus_mode(FOCUS_NONE);
	section_header->connect("pressed", this, "_on_header_pressed");

	tasks_container = memnew(HFlowContainer);
	add_child(tasks_container);
}

TaskSection::~TaskSection() {
}

////////////////////////////// TaskSection  ////////////////////////////////////

//////////////////////////////  TaskPanel  /////////////////////////////////////

void TaskPanel::_on_task_button_pressed(const StringName &p_task) {
	emit_signal("task_selected", p_task);
}

void TaskPanel::_on_filter_text_changed(String p_text) {
	for (int i = 0; i < sections->get_child_count(); i++) {
		TaskSection *sec = Object::cast_to<TaskSection>(sections->get_child(i));
		sec->set_filter(p_text);
	}
}

void TaskPanel::refresh() {
	filter_edit->set_right_icon(get_icon("Search", "EditorIcons"));

	Set<String> collapsed_sections;
	if (sections->get_child_count() == 0) {
		// Restore collapsed state from config.
		ConfigFile conf;
		String conf_path = EditorSettings::get_singleton()->get_project_settings_dir().plus_file("limbo_ai.cfg");
		if (conf.load(conf_path) == OK) {
			Variant value = conf.get_value("bt_editor", "collapsed_sections", Array());
			if (value.is_array()) {
				Array arr = value;
				for (int i = 0; i < arr.size(); i++) {
					if (arr[i].get_type() == Variant::STRING) {
						collapsed_sections.insert(arr[i]);
					}
				}
			}
		}
	} else {
		for (int i = 0; i < sections->get_child_count(); i++) {
			TaskSection *sec = Object::cast_to<TaskSection>(sections->get_child(i));
			if (sec->is_collapsed()) {
				collapsed_sections.insert(sec->get_category_name());
			}
			sections->get_child(i)->queue_delete();
		}
	}

	HashMap<String, List<String>> categories;

	categories["Composites"] = List<String>();
	_populate_core_tasks_from_class("BTComposite", &categories["Composites"]);

	categories["Actions"] = List<String>();
	_populate_core_tasks_from_class("BTAction", &categories["Actions"]);

	categories["Decorators"] = List<String>();
	_populate_core_tasks_from_class("BTDecorator", &categories["Decorators"]);

	categories["Conditions"] = List<String>();
	_populate_core_tasks_from_class("BTCondition", &categories["Conditions"]);

	categories["User"] = List<String>();

	String dir1 = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_1");
	_populate_from_user_dir(dir1, &categories);

	String dir2 = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_2");
	_populate_from_user_dir(dir2, &categories);

	String dir3 = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_3");
	_populate_from_user_dir(dir3, &categories);

	List<String> keys;
	categories.get_key_list(&keys);
	keys.sort();
	for (List<String>::Element *E = keys.front(); E; E = E->next()) {
		String cat = E->get();
		List<String> task_list = categories.get(cat);

		if (task_list.size() == 0) {
			continue;
		}

		TaskSection *sec = memnew(TaskSection(cat, editor));
		for (List<String>::Element *E = task_list.front(); E; E = E->next()) {
			String meta = E->get();
			String tname;
			Ref<Texture> icon;
			icon = LimboAIEditor::get_task_icon(meta);
			tname = meta.begins_with("res:") ? meta.get_file().get_basename().trim_prefix("BT") : meta.trim_prefix("BT");
			sec->add_task_button(tname, icon, meta);
		}
		sec->set_filter("");
		sec->connect("task_button_pressed", this, "_on_task_button_pressed");
		sections->add_child(sec);
		sec->set_collapsed(collapsed_sections.has(cat));
	}
}

void TaskPanel::_populate_core_tasks_from_class(const StringName &p_base_class, List<String> *p_task_classes) {
	List<StringName> inheriters;
	ClassDB::get_inheriters_from_class(p_base_class, &inheriters);

	for (List<StringName>::Element *E = inheriters.front(); E; E = E->next()) {
		p_task_classes->push_back(E->get());
	}
}

void TaskPanel::_populate_from_user_dir(String p_path, HashMap<String, List<String>> *p_categories) {
	if (p_path.empty()) {
		return;
	}
	DirAccess *dir = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	if (dir->change_dir(p_path) == OK) {
		dir->list_dir_begin();
		String fn = dir->get_next();
		while (!fn.empty()) {
			if (dir->current_is_dir() && fn != "..") {
				String full_path;
				String category;
				if (fn == ".") {
					full_path = p_path;
					category = "User";
				} else {
					full_path = p_path.plus_file(fn);
					category = fn.capitalize();
				}

				if (!p_categories->has(category)) {
					p_categories->set(category, List<String>());
				}

				_populate_scripted_tasks_from_dir(full_path, &p_categories->get(category));
			}
			fn = dir->get_next();
		}
		dir->list_dir_end();
	} else {
		ERR_FAIL_MSG(vformat("Failed to list \"%s\" directory.", p_path));
	}
	memdelete(dir);
}

void TaskPanel::_populate_scripted_tasks_from_dir(String p_path, List<String> *p_task_classes) {
	if (p_path.empty()) {
		return;
	}
	DirAccess *dir = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	if (dir->change_dir(p_path) == OK) {
		dir->list_dir_begin();
		String fn = dir->get_next();
		while (!fn.empty()) {
			if (fn.ends_with(".gd")) {
				String full_path = p_path.plus_file(fn);
				p_task_classes->push_back(full_path);
			}
			fn = dir->get_next();
		}
		dir->list_dir_end();
	} else {
		ERR_FAIL_MSG(vformat("Failed to list \"%s\" directory.", p_path));
	}
	memdelete(dir);
}

void TaskPanel::_notification(int p_what) {
	if (p_what == NOTIFICATION_EXIT_TREE) {
		if (sections->get_child_count() == 0) {
			return;
		}
		Array collapsed_sections;
		for (int i = 0; i < sections->get_child_count(); i++) {
			TaskSection *sec = Object::cast_to<TaskSection>(sections->get_child(i));
			if (sec->is_collapsed()) {
				collapsed_sections.push_back(sec->get_category_name());
			}
		}
		ConfigFile conf;
		String conf_path = EditorSettings::get_singleton()->get_project_settings_dir().plus_file("limbo_ai.cfg");
		conf.load(conf_path);
		conf.set_value("bt_editor", "collapsed_sections", collapsed_sections);
		conf.save(conf_path);
	}
}

void TaskPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("refresh"), &TaskPanel::refresh);
	ClassDB::bind_method(D_METHOD("_on_task_button_pressed"), &TaskPanel::_on_task_button_pressed);
	ClassDB::bind_method(D_METHOD("_on_filter_text_changed"), &TaskPanel::_on_filter_text_changed);

	ADD_SIGNAL(MethodInfo("task_selected"));
}

TaskPanel::TaskPanel(EditorNode *p_editor) {
	editor = p_editor;

	VBoxContainer *vb = memnew(VBoxContainer);
	add_child(vb);

	filter_edit = memnew(LineEdit);
	vb->add_child(filter_edit);
	filter_edit->set_clear_button_enabled(true);
	filter_edit->connect("text_changed", this, "_on_filter_text_changed");

	ScrollContainer *sc = memnew(ScrollContainer);
	vb->add_child(sc);
	sc->set_h_size_flags(SIZE_EXPAND_FILL);
	sc->set_v_size_flags(SIZE_EXPAND_FILL);

	sections = memnew(VBoxContainer);
	sc->add_child(sections);
	sections->set_h_size_flags(SIZE_EXPAND_FILL);
	sections->set_v_size_flags(SIZE_EXPAND_FILL);
}

TaskPanel::~TaskPanel() {
}

//////////////////////////////  TaskPanel  /////////////////////////////////////

////////////////////////////  LimboAIEditor  ///////////////////////////////////

void LimboAIEditor::_add_task(const Ref<BTTask> &p_task) {
	ERR_FAIL_COND(p_task.is_null());
	ERR_FAIL_COND(task_tree->get_bt().is_null());
	Ref<BTTask> parent = task_tree->get_selected();
	if (parent.is_null()) {
		parent = task_tree->get_bt()->get_root_task();
	}
	if (parent.is_null()) {
		task_tree->get_bt()->set_root_task(p_task);
	} else {
		parent->add_child(p_task);
	}
	_mark_as_dirty(true);
	task_tree->update_tree();
}

void LimboAIEditor::_update_header() const {
	String text = task_tree->get_bt()->get_path();
	if (text.empty()) {
		text = TTR("New Behavior Tree");
	} else if (dirty.has(task_tree->get_bt())) {
		text += "(*)";
	}

	header->set_text(text);
	header->set_icon(editor->get_object_icon(task_tree->get_bt().ptr(), "BehaviorTree"));
}

void LimboAIEditor::_update_history_buttons() {
	history_back->set_disabled(idx_history == 0);
	history_forward->set_disabled(idx_history == (history.size() - 1));
}

void LimboAIEditor::_new_bt() {
	BehaviorTree *bt = memnew(BehaviorTree);
	bt->set_root_task(memnew(BTSelector));
	editor->edit_resource(bt);
}

void LimboAIEditor::_save_bt(String p_path) {
	ERR_FAIL_COND_MSG(p_path.empty(), "Empty p_path");
	ERR_FAIL_COND_MSG(task_tree->get_bt().is_null(), "Behavior Tree is null.");
	task_tree->get_bt()->set_path(p_path, true);
	ResourceSaver::save(p_path, task_tree->get_bt(), ResourceSaver::FLAG_CHANGE_PATH);
	_update_header();
	_mark_as_dirty(false);
}

void LimboAIEditor::_load_bt(String p_path) {
	ERR_FAIL_COND_MSG(p_path.empty(), "Empty p_path");
	Ref<BehaviorTree> bt = ResourceLoader::load(p_path, "BehaviorTree");

	if (history.find(bt) != -1) {
		history.erase(bt);
		history.push_back(bt);
	}

	editor->edit_resource(bt);
}

void LimboAIEditor::edit_bt(Ref<BehaviorTree> p_behavior_tree, bool p_force_refresh) {
	ERR_FAIL_COND_MSG(p_behavior_tree.is_null(), "p_behavior_tree is null");

	if (!p_force_refresh && task_tree->get_bt() == p_behavior_tree) {
		return;
	}

	task_tree->load_bt(p_behavior_tree);

	int idx = history.find(p_behavior_tree);
	if (idx != -1) {
		idx_history = idx;
	} else {
		history.push_back(p_behavior_tree);
		idx_history = history.size() - 1;
	}

	usage_hint->hide();
	task_tree->show();
	task_panel->show();

	_update_history_buttons();
	_update_header();
}

void LimboAIEditor::_mark_as_dirty(bool p_dirty) {
	Ref<BehaviorTree> bt = task_tree->get_bt();
	if (p_dirty && !dirty.has(bt)) {
		dirty.insert(bt);
		_update_header();
	} else if (p_dirty == false && dirty.has(bt)) {
		dirty.erase(bt);
		_update_header();
	}
}

void LimboAIEditor::_on_tree_rmb(const Vector2 &p_menu_pos) {
	menu->set_size(Size2(1, 1));
	menu->set_position(p_menu_pos);

	menu->clear();
	menu->add_icon_item(get_icon("Remove", "EditorIcons"), TTR("Remove"), ACTION_REMOVE);
	menu->add_separator();
	menu->add_icon_item(get_icon("MoveUp", "EditorIcons"), TTR("Move Up"), ACTION_MOVE_UP);
	menu->add_icon_item(get_icon("MoveDown", "EditorIcons"), TTR("Move Down"), ACTION_MOVE_DOWN);
	menu->add_icon_item(get_icon("Duplicate", "EditorIcons"), TTR("Duplicate"), ACTION_DUPLICATE);
	menu->add_icon_item(get_icon("NewRoot", "EditorIcons"), TTR("Make Root"), ACTION_MAKE_ROOT);

	menu->popup();
}

void LimboAIEditor::_on_action_selected(int p_id) {
	switch (p_id) {
		case ACTION_REMOVE: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid()) {
				if (sel->get_parent().is_null()) {
					task_tree->get_bt()->set_root_task(nullptr);
				} else {
					sel->get_parent()->remove_child(sel);
				}
				task_tree->update_tree();
				editor->edit_node(nullptr);
				_mark_as_dirty(true);
			}
		} break;
		case ACTION_MOVE_UP: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid() && sel->get_parent().is_valid()) {
				Ref<BTTask> parent = sel->get_parent();
				int idx = parent->get_child_index(sel);
				if (idx > 0 && idx < parent->get_child_count()) {
					parent->remove_child(sel);
					parent->add_child_at_index(sel, idx - 1);
					task_tree->update_tree();
					_mark_as_dirty(true);
				}
			}
		} break;
		case ACTION_MOVE_DOWN: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid() && sel->get_parent().is_valid()) {
				Ref<BTTask> parent = sel->get_parent();
				int idx = parent->get_child_index(sel);
				if (idx >= 0 && idx < (parent->get_child_count() - 1)) {
					parent->remove_child(sel);
					parent->add_child_at_index(sel, idx + 1);
					task_tree->update_tree();
					_mark_as_dirty(true);
				}
			}
		} break;
		case ACTION_DUPLICATE: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid()) {
				Ref<BTTask> parent = sel->get_parent();
				if (parent.is_null()) {
					parent = sel;
				}
				parent->add_child_at_index(sel->clone(), parent->get_child_index(sel) + 1);
				task_tree->update_tree();
				_mark_as_dirty(true);
			}
		} break;
		case ACTION_MAKE_ROOT: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid() && task_tree->get_bt()->get_root_task() != sel) {
				Ref<BTTask> parent = sel->get_parent();
				ERR_FAIL_COND(parent.is_null());
				parent->remove_child(sel);
				Ref<BTTask> old_root = task_tree->get_bt()->get_root_task();
				task_tree->get_bt()->set_root_task(sel);
				sel->add_child(old_root);
				task_tree->update_tree();
				_mark_as_dirty(true);
			}
		} break;
	}
}

void LimboAIEditor::_on_tree_task_selected(const Ref<BTTask> &p_task) const {
	editor->edit_resource(p_task);
}

void LimboAIEditor::_on_panel_task_selected(String p_task) {
	if (p_task.begins_with("res:")) {
		Ref<Script> script = ResourceLoader::load(p_task, "Script");
		ERR_FAIL_COND_MSG(script.is_null() || !script->is_valid(), vformat("LimboAI: Failed to instance task. Bad script: %s", p_task));
		Variant inst = ClassDB::instance(script->get_instance_base_type());
		ERR_FAIL_COND_MSG(inst.is_zero(), vformat("LimboAI: Failed to instance base type \"%s\".", script->get_instance_base_type()));

		if (unlikely(!((Object *)inst)->is_class("BTTask"))) {
			if (!inst.is_ref()) {
				memdelete((Object *)inst);
			}
			ERR_PRINT(vformat("LimboAI: Failed to instance task. Script is not a BTTask: %s", p_task));
			return;
		}

		if (inst && script.is_valid()) {
			((Object *)inst)->set_script(script.get_ref_ptr());
			_add_task(Variant(inst));
		}
	} else {
		_add_task(Ref<BTTask>(ClassDB::instance(p_task)));
	}
}

void LimboAIEditor::_on_visibility_changed() const {
	if (is_visible()) {
		Ref<BTTask> sel = task_tree->get_selected();
		if (sel.is_valid()) {
			editor->edit_resource(sel);
		} else if (task_tree->get_bt().is_valid() && editor->get_inspector()->get_edited_object() != task_tree->get_bt().ptr()) {
			editor->edit_resource(task_tree->get_bt());
		}

		task_panel->refresh();
	}
}

void LimboAIEditor::_on_header_pressed() const {
	_update_header();
	task_tree->deselect();
	editor->edit_resource(task_tree->get_bt());
}

void LimboAIEditor::_on_save_pressed() {
	if (task_tree->get_bt().is_null()) {
		return;
	}
	String path = task_tree->get_bt()->get_path();
	if (path.empty()) {
		save_dialog->popup_centered_ratio();
	} else {
		_save_bt(path);
	}
}

void LimboAIEditor::_on_history_back() {
	idx_history = MAX(idx_history - 1, 0);
	editor->edit_resource(history[idx_history]);
}

void LimboAIEditor::_on_history_forward() {
	idx_history = MIN(idx_history + 1, history.size() - 1);
	editor->edit_resource(history[idx_history]);
}

void LimboAIEditor::_on_task_dragged(Ref<BTTask> p_task, Ref<BTTask> p_to_task, int p_type) {
	if (p_task == p_to_task) {
		return;
	}
	if (p_type == 0) {
		p_task->get_parent()->remove_child(p_task);
		p_to_task->add_child(p_task);
		task_tree->update_tree();
		_mark_as_dirty(true);
	} else if (p_type == -1 && p_to_task->get_parent().is_valid()) {
		p_task->get_parent()->remove_child(p_task);
		p_to_task->get_parent()->add_child_at_index(p_task, p_to_task->get_parent()->get_child_index(p_to_task));
		task_tree->update_tree();
		_mark_as_dirty(true);
	} else if (p_type == 1 && p_to_task->get_parent().is_valid()) {
		p_task->get_parent()->remove_child(p_task);
		p_to_task->get_parent()->add_child_at_index(p_task, p_to_task->get_parent()->get_child_index(p_to_task) + 1);
		task_tree->update_tree();
		_mark_as_dirty(true);
	}
}

void LimboAIEditor::_on_resources_reload(const Vector<String> &p_resources) {
	for (int i = 0; i < p_resources.size(); i++) {
		if (!ResourceCache::has(p_resources[i])) {
			continue;
		}

		String res_type = ResourceLoader::get_resource_type(p_resources[i]);
		if (res_type == "BehaviorTree") {
			for (int j = 0; j < history.size(); j++) {
				if (history.get(j)->get_path() == p_resources[i]) {
					disk_changed_files.insert(p_resources[i]);
				}
			}
		}
	}

	if (disk_changed_files.size() > 0) {
		disk_changed_list->clear();
		disk_changed_list->set_hide_root(true);
		disk_changed_list->create_item();
		for (Set<String>::Element *E = disk_changed_files.front(); E; E = E->next()) {
			// for (int i = 0; i < disk_changed_files.size(); i++) {
			TreeItem *ti = disk_changed_list->create_item();
			ti->set_text(0, E->get());
		}

		if (!is_visible()) {
			EditorNode::get_singleton()->select_editor_by_name("LimboAI");
		}
		disk_changed->call_deferred("popup_centered_ratio", 0.5);
	}
}

void LimboAIEditor::_reload_modified() {
	for (Set<String>::Element *E = disk_changed_files.front(); E; E = E->next()) {
		for (int j = 0; j < history.size(); j++) {
			if (history.get(j)->get_path() == E->get()) {
				dirty.erase(history.get(j));
				history.get(j)->get_root_task()->clear_internal_resource_paths();
				history.get(j)->reload_from_file();
				if (j == idx_history) {
					edit_bt(history.get(j), true);
				}
			}
		}
	}
	disk_changed_files.clear();
}

void LimboAIEditor::_resave_modified(String _str) {
	for (Set<String>::Element *E = disk_changed_files.front(); E; E = E->next()) {
		for (int j = 0; j < history.size(); j++) {
			if (history.get(j)->get_path() == E->get()) {
				ResourceSaver::save(history.get(j)->get_path(), history.get(j));
			}
		}
	}
	disk_changed->hide();
	disk_changed_files.clear();
}

void LimboAIEditor::apply_changes() {
	for (int i = 0; i < history.size(); i++) {
		Ref<BehaviorTree> bt = history.get(i);
		String path = bt->get_path();
		if (ResourceLoader::exists(path)) {
			ResourceSaver::save(path, bt);
		}
		dirty.clear();
		_update_header();
	}
}

Ref<Texture> LimboAIEditor::get_task_icon(String p_script_path_or_class) {
	// TODO: Implement caching.
	String base_type = p_script_path_or_class;
	if (p_script_path_or_class.begins_with("res:")) {
		Ref<Script> script = ResourceLoader::load(p_script_path_or_class, "Script");
		Ref<Script> base_script = script;
		while (base_script.is_valid()) {
			StringName name = EditorNode::get_editor_data().script_class_get_name(base_script->get_path());
			String icon_path = EditorNode::get_editor_data().script_class_get_icon_path(name);
			if (!icon_path.empty()) {
				Ref<Image> img = memnew(Image);
				Error err = ImageLoader::load_image(icon_path, img);
				if (err == OK) {
					Ref<ImageTexture> icon = memnew(ImageTexture);
					img->resize(16 * EDSCALE, 16 * EDSCALE, Image::INTERPOLATE_LANCZOS);
					icon->create_from_image(img);
					return icon;
				}
			}
			base_script = base_script->get_base_script();
		}
		base_type = script->get_instance_base_type();
	}

	// TODO: Walk inheritance tree until icon is found.
	return EditorNode::get_singleton()->get_class_icon(base_type, "BTTask");
}

void LimboAIEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			ConfigFile conf;
			String conf_path = EditorSettings::get_singleton()->get_project_settings_dir().plus_file("limbo_ai.cfg");
			if (conf.load(conf_path) == OK) {
				hsc->set_split_offset(conf.get_value("bt_editor", "bteditor_hsplit", hsc->get_split_offset()));
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			ConfigFile conf;
			String conf_path = EditorSettings::get_singleton()->get_project_settings_dir().plus_file("limbo_ai.cfg");
			conf.load(conf_path);
			conf.set_value("bt_editor", "bteditor_hsplit", hsc->get_split_offset());
			conf.save(conf_path);
		} break;
	}
}

void LimboAIEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_add_task", "p_task"), &LimboAIEditor::_add_task);
	ClassDB::bind_method(D_METHOD("_add_task_with_prototype", "p_prototype"), &LimboAIEditor::_add_task_with_prototype);
	ClassDB::bind_method(D_METHOD("_on_tree_rmb"), &LimboAIEditor::_on_tree_rmb);
	ClassDB::bind_method(D_METHOD("_on_action_selected", "p_id"), &LimboAIEditor::_on_action_selected);
	ClassDB::bind_method(D_METHOD("_on_tree_task_selected", "p_task"), &LimboAIEditor::_on_tree_task_selected);
	ClassDB::bind_method(D_METHOD("_on_panel_task_selected", "p_task"), &LimboAIEditor::_on_panel_task_selected);
	ClassDB::bind_method(D_METHOD("_on_visibility_changed"), &LimboAIEditor::_on_visibility_changed);
	ClassDB::bind_method(D_METHOD("_on_header_pressed"), &LimboAIEditor::_on_header_pressed);
	ClassDB::bind_method(D_METHOD("_on_save_pressed"), &LimboAIEditor::_on_save_pressed);
	ClassDB::bind_method(D_METHOD("_on_history_back"), &LimboAIEditor::_on_history_back);
	ClassDB::bind_method(D_METHOD("_on_history_forward"), &LimboAIEditor::_on_history_forward);
	ClassDB::bind_method(D_METHOD("_on_task_dragged", "p_task", "p_to_task", "p_type"), &LimboAIEditor::_on_task_dragged);
	ClassDB::bind_method(D_METHOD("_new_bt"), &LimboAIEditor::_new_bt);
	ClassDB::bind_method(D_METHOD("_save_bt", "p_path"), &LimboAIEditor::_save_bt);
	ClassDB::bind_method(D_METHOD("_load_bt", "p_path"), &LimboAIEditor::_load_bt);
	ClassDB::bind_method(D_METHOD("edit_bt", "p_behavior_tree", "p_force_refresh"), &LimboAIEditor::edit_bt, Variant(false));
	ClassDB::bind_method(D_METHOD("_on_resources_reload"), &LimboAIEditor::_on_resources_reload);
	ClassDB::bind_method(D_METHOD("_reload_modified"), &LimboAIEditor::_reload_modified);
	ClassDB::bind_method(D_METHOD("_resave_modified"), &LimboAIEditor::_resave_modified);
}

LimboAIEditor::LimboAIEditor(EditorNode *p_editor) {
	editor = p_editor;

	save_dialog = memnew(FileDialog);
	add_child(save_dialog);
	save_dialog->set_mode(FileDialog::MODE_SAVE_FILE);
	save_dialog->set_title("Save Behavior Tree");
	save_dialog->add_filter("*.tres");
	save_dialog->connect("file_selected", this, "_save_bt");
	save_dialog->hide();

	load_dialog = memnew(FileDialog);
	add_child(load_dialog);
	load_dialog->set_mode(FileDialog::MODE_OPEN_FILE);
	load_dialog->set_title("Load Behavior Tree");
	load_dialog->add_filter("*.tres");
	load_dialog->connect("file_selected", this, "_load_bt");
	load_dialog->hide();

	VBoxContainer *vb = memnew(VBoxContainer);
	vb->set_anchor(MARGIN_RIGHT, ANCHOR_END);
	vb->set_anchor(MARGIN_BOTTOM, ANCHOR_END);
	add_child(vb);

	HBoxContainer *panel = memnew(HBoxContainer);
	vb->add_child(panel);

	Button *selector_btn = memnew(Button);
	selector_btn->set_text(TTR("Selector"));
	selector_btn->set_tooltip(TTR("Add Selector task."));
	selector_btn->set_icon(editor->get_class_icon("BTSelector"));
	selector_btn->set_flat(true);
	selector_btn->set_focus_mode(Control::FOCUS_NONE);
	selector_btn->connect("pressed", this, "_add_task_with_prototype", varray(Ref<BTTask>(memnew(BTSelector))));
	panel->add_child(selector_btn);

	Button *sequence_btn = memnew(Button);
	sequence_btn->set_text(TTR("Sequence"));
	sequence_btn->set_tooltip(TTR("Add Sequence task."));
	sequence_btn->set_icon(editor->get_class_icon("BTSequence"));
	sequence_btn->set_flat(true);
	sequence_btn->set_focus_mode(Control::FOCUS_NONE);
	sequence_btn->connect("pressed", this, "_add_task_with_prototype", varray(Ref<BTTask>(memnew(BTSequence))));
	panel->add_child(sequence_btn);

	Button *parallel_btn = memnew(Button);
	parallel_btn->set_text(TTR("Parallel"));
	parallel_btn->set_tooltip(TTR("Add Parallel task."));
	parallel_btn->set_icon(editor->get_class_icon("BTParallel"));
	parallel_btn->set_flat(true);
	parallel_btn->set_focus_mode(Control::FOCUS_NONE);
	parallel_btn->connect("pressed", this, "_add_task_with_prototype", varray(Ref<BTTask>(memnew(BTParallel))));
	panel->add_child(parallel_btn);

	panel->add_child(memnew(VSeparator));

	Button *new_btn = memnew(Button);
	panel->add_child(new_btn);
	new_btn->set_text(TTR("New"));
	new_btn->set_tooltip(TTR("Create new behavior tree."));
	new_btn->set_icon(editor->get_gui_base()->get_icon("New", "EditorIcons"));
	new_btn->set_flat(true);
	new_btn->set_focus_mode(Control::FOCUS_NONE);
	new_btn->connect("pressed", this, "_new_bt");

	Button *load_btn = memnew(Button);
	panel->add_child(load_btn);
	load_btn->set_text(TTR("Load"));
	load_btn->set_tooltip(TTR("Load behavior tree."));
	load_btn->set_icon(editor->get_gui_base()->get_icon("Load", "EditorIcons"));
	load_btn->set_flat(true);
	load_btn->set_focus_mode(Control::FOCUS_NONE);
	load_btn->connect("pressed", load_dialog, "popup_centered_ratio");

	Button *save_btn = memnew(Button);
	panel->add_child(save_btn);
	save_btn->set_text(TTR("Save"));
	save_btn->set_tooltip(TTR("Save current behavior tree."));
	save_btn->set_icon(editor->get_gui_base()->get_icon("Save", "EditorIcons"));
	save_btn->set_flat(true);
	save_btn->set_focus_mode(Control::FOCUS_NONE);
	save_btn->connect("pressed", this, "_on_save_pressed");

	panel->add_child(memnew(VSeparator));

	Button *new_script_btn = memnew(Button);
	panel->add_child(new_script_btn);
	new_script_btn->set_text(TTR("New Task"));
	new_script_btn->set_tooltip(TTR("Create new task script and edit it."));
	new_script_btn->set_icon(editor->get_gui_base()->get_icon("ScriptCreate", "EditorIcons"));
	new_script_btn->set_flat(true);
	new_script_btn->set_focus_mode(Control::FOCUS_NONE);

	HBoxContainer *nav = memnew(HBoxContainer);
	panel->add_child(nav);
	nav->set_h_size_flags(SIZE_EXPAND | SIZE_SHRINK_END);

	history_back = memnew(Button);
	history_back->set_icon(editor->get_gui_base()->get_icon("Back", "EditorIcons"));
	history_back->set_flat(true);
	history_back->set_focus_mode(FOCUS_NONE);
	history_back->connect("pressed", this, "_on_history_back");
	nav->add_child(history_back);

	history_forward = memnew(Button);
	history_forward->set_icon(editor->get_gui_base()->get_icon("Forward", "EditorIcons"));
	history_forward->set_flat(true);
	history_forward->set_focus_mode(FOCUS_NONE);
	history_forward->connect("pressed", this, "_on_history_forward");
	nav->add_child(history_forward);

	header = memnew(Button);
	vb->add_child(header);
	header->set_text_align(Button::ALIGN_LEFT);
	header->add_constant_override("hseparation", 8);
	header->connect("pressed", this, "_on_header_pressed");

	hsc = memnew(HSplitContainer);
	vb->add_child(hsc);
	hsc->set_h_size_flags(SIZE_EXPAND_FILL);
	hsc->set_v_size_flags(SIZE_EXPAND_FILL);

	task_tree = memnew(TaskTree);
	hsc->add_child(task_tree);
	task_tree->set_v_size_flags(SIZE_EXPAND_FILL);
	task_tree->set_h_size_flags(SIZE_EXPAND_FILL);
	task_tree->connect("rmb_pressed", this, "_on_tree_rmb");
	task_tree->connect("task_selected", this, "_on_tree_task_selected");
	task_tree->connect("visibility_changed", this, "_on_visibility_changed");
	task_tree->connect("task_dragged", this, "_on_task_dragged");
	task_tree->hide();

	usage_hint = memnew(Panel);
	usage_hint->set_v_size_flags(SIZE_EXPAND_FILL);
	usage_hint->set_h_size_flags(SIZE_EXPAND_FILL);
	hsc->add_child(usage_hint);
	Label *usage_label = memnew(Label);
	usage_label->set_anchor(MARGIN_RIGHT, 1);
	usage_label->set_anchor(MARGIN_BOTTOM, 1);
	usage_label->set_align(Label::ALIGN_CENTER);
	usage_label->set_valign(Label::VALIGN_CENTER);
	usage_label->set_text(TTR("Create a new or load an existing behavior tree."));
	usage_hint->add_child(usage_label);

	task_panel = memnew(TaskPanel(p_editor));
	hsc->add_child(task_panel);
	hsc->set_split_offset(-300);
	task_panel->connect("task_selected", this, "_on_panel_task_selected");
	task_panel->hide();

	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", this, "_on_action_selected");
	menu->set_hide_on_window_lose_focus(true);

	disk_changed = memnew(ConfirmationDialog);
	{
		VBoxContainer *vbc = memnew(VBoxContainer);
		disk_changed->add_child(vbc);

		Label *dl = memnew(Label);
		dl->set_text(TTR("The following BehaviorTree resources are newer on disk.\nWhat action should be taken?"));
		vbc->add_child(dl);

		disk_changed_list = memnew(Tree);
		vbc->add_child(disk_changed_list);
		disk_changed_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);

		disk_changed->get_ok()->set_text(TTR("Reload"));
		disk_changed->connect("confirmed", this, "_reload_modified");

		disk_changed->add_button(TTR("Resave"), !OS::get_singleton()->get_swap_ok_cancel(), "resave");
		disk_changed->connect("custom_action", this, "_resave_modified");
	}
	editor->get_gui_base()->add_child(disk_changed);
	// disk_changed->hide();

	GLOBAL_DEF("limbo_ai/behavior_tree/behavior_tree_default_dir", "res://ai/trees");
	ProjectSettings::get_singleton()->set_custom_property_info("limbo_ai/behavior_tree/behavior_tree_default_dir",
			PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/behavior_tree_default_dir", PROPERTY_HINT_DIR));
	GLOBAL_DEF("limbo_ai/behavior_tree/user_task_dir_1", "res://ai/tasks");
	ProjectSettings::get_singleton()->set_custom_property_info("limbo_ai/behavior_tree/user_task_dir_1",
			PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/user_task_dir_1", PROPERTY_HINT_DIR));
	GLOBAL_DEF("limbo_ai/behavior_tree/user_task_dir_2", "");
	ProjectSettings::get_singleton()->set_custom_property_info("limbo_ai/behavior_tree/user_task_dir_2",
			PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/user_task_dir_2", PROPERTY_HINT_DIR));
	GLOBAL_DEF("limbo_ai/behavior_tree/user_task_dir_3", "");
	ProjectSettings::get_singleton()->set_custom_property_info("limbo_ai/behavior_tree/user_task_dir_3",
			PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/user_task_dir_3", PROPERTY_HINT_DIR));

	save_dialog->set_current_dir(GLOBAL_GET("limbo_ai/behavior_tree/behavior_tree_default_dir"));
	load_dialog->set_current_dir(GLOBAL_GET("limbo_ai/behavior_tree/behavior_tree_default_dir"));
	new_script_btn->connect("pressed", ScriptEditor::get_singleton(), "open_script_create_dialog",
			varray("BTAction", String(GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_1")).plus_file("new_task")));

	EditorFileSystem::get_singleton()->connect("resources_reload", this, "_on_resources_reload");
}

LimboAIEditor::~LimboAIEditor() {
}

////////////////////////////  LimboAIEditor  ///////////////////////////////////

/////////////////////////  LimboAIEditorPlugin  ////////////////////////////////

const Ref<Texture> LimboAIEditorPlugin::get_icon() const {
	return editor->get_gui_base()->get_icon("LimboAIEditor", "EditorIcons");
}

void LimboAIEditorPlugin::apply_changes() {
	limbo_ai_editor->apply_changes();
}

void LimboAIEditorPlugin::_notification(int p_notification) {
	// print_line(vformat("NOTIFICATION: %d", p_notification));
}

void LimboAIEditorPlugin::make_visible(bool p_visible) {
	limbo_ai_editor->set_visible(p_visible);
}

void LimboAIEditorPlugin::edit(Object *p_object) {
	if (Object::cast_to<BehaviorTree>(p_object)) {
		limbo_ai_editor->edit_bt(Object::cast_to<BehaviorTree>(p_object));
	}
}

bool LimboAIEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("BehaviorTree");
}

LimboAIEditorPlugin::LimboAIEditorPlugin(EditorNode *p_editor) {
	editor = p_editor;
	limbo_ai_editor = memnew(LimboAIEditor(p_editor));
	limbo_ai_editor->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	editor->get_viewport()->add_child(limbo_ai_editor);
	limbo_ai_editor->hide();
}

LimboAIEditorPlugin::~LimboAIEditorPlugin() {
}

#endif // TOOLS_ENABLED