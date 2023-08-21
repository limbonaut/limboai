/**
 * limbo_ai_editor_plugin.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#include "limbo_ai_editor_plugin.h"

#include "modules/limboai/bt/behavior_tree.h"
#include "modules/limboai/bt/tasks/bt_action.h"
#include "modules/limboai/bt/tasks/bt_comment.h"
#include "modules/limboai/bt/tasks/bt_task.h"
#include "modules/limboai/bt/tasks/composites/bt_parallel.h"
#include "modules/limboai/bt/tasks/composites/bt_selector.h"
#include "modules/limboai/bt/tasks/composites/bt_sequence.h"
#include "modules/limboai/editor/debugger/limbo_debugger_plugin.h"
#include "modules/limboai/util/limbo_utility.h"

#include "core/config/project_settings.h"
#include "core/error/error_list.h"
#include "core/error/error_macros.h"
#include "core/io/config_file.h"
#include "core/io/dir_access.h"
#include "core/io/image_loader.h"
#include "core/io/resource.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/math/math_defs.h"
#include "core/math/vector2.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/script_language.h"
#include "core/object/undo_redo.h"
#include "core/os/memory.h"
#include "core/string/print_string.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/list.h"
#include "core/templates/vector.h"
#include "core/typedefs.h"
#include "core/variant/array.h"
#include "core/variant/callable.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "editor/debugger/editor_debugger_node.h"
#include "editor/debugger/script_editor_debugger.h"
#include "editor/editor_file_system.h"
#include "editor/editor_inspector.h"
#include "editor/editor_node.h"
#include "editor/editor_paths.h"
#include "editor/editor_plugin.h"
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/inspector_dock.h"
#include "editor/plugins/script_editor_plugin.h"
#include "editor/project_settings_editor.h"
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
#include "servers/display_server.h"

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

//**** TaskTree ^

//**** TaskSection

void TaskSection::_on_task_button_pressed(const String &p_task) {
	emit_signal(SNAME("task_button_pressed"), p_task);
}

void TaskSection::_on_task_button_gui_input(const Ref<InputEvent> &p_event, const String &p_task) {
	if (!p_event->is_pressed()) {
		return;
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == MouseButton::RIGHT) {
		emit_signal(SNAME("task_button_rmb"), p_task);
	}
}

void TaskSection::_on_header_pressed() {
	set_collapsed(!is_collapsed());
}

void TaskSection::set_filter(String p_filter_text) {
	int num_hidden = 0;
	if (p_filter_text.is_empty()) {
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
	btn->add_theme_constant_override(SNAME("icon_max_width"), 16 * EDSCALE); // Force user icons to  be of the proper size.
	btn->connect(SNAME("pressed"), callable_mp(this, &TaskSection::_on_task_button_pressed).bind(p_meta));
	btn->connect(SNAME("gui_input"), callable_mp(this, &TaskSection::_on_task_button_gui_input).bind(p_meta));
	tasks_container->add_child(btn);
}

void TaskSection::set_collapsed(bool p_collapsed) {
	tasks_container->set_visible(!p_collapsed);
	section_header->set_icon(p_collapsed ? get_theme_icon(SNAME("GuiTreeArrowRight"), SNAME("EditorIcons")) : get_theme_icon(SNAME("GuiTreeArrowDown"), SNAME("EditorIcons")));
}

bool TaskSection::is_collapsed() const {
	return !tasks_container->is_visible();
}

void TaskSection::_notification(int p_what) {
	if (p_what == NOTIFICATION_THEME_CHANGED) {
		section_header->set_icon(is_collapsed() ? get_theme_icon(SNAME("GuiTreeArrowRight"), SNAME("EditorIcons")) : get_theme_icon(SNAME("GuiTreeArrowDown"), SNAME("EditorIcons")));
		section_header->add_theme_font_override(SNAME("font"), get_theme_font(SNAME("bold"), SNAME("EditorFonts")));
	}
}

void TaskSection::_bind_methods() {
	ADD_SIGNAL(MethodInfo("task_button_pressed"));
	ADD_SIGNAL(MethodInfo("task_button_rmb"));
}

TaskSection::TaskSection(String p_category_name) {
	section_header = memnew(Button);
	add_child(section_header);
	section_header->set_text(p_category_name);
	section_header->set_focus_mode(FOCUS_NONE);
	section_header->connect("pressed", callable_mp(this, &TaskSection::_on_header_pressed));

	tasks_container = memnew(HFlowContainer);
	add_child(tasks_container);
}

TaskSection::~TaskSection() {
}

//**** TaskSection ^

//**** TaskPanel

void TaskPanel::_menu_action_selected(int p_id) {
	ERR_FAIL_COND(context_task.is_empty());
	switch (p_id) {
		case MENU_OPEN_DOC: {
			String help_class;
			if (context_task.begins_with("res://")) {
				Ref<Script> s = ResourceLoader::load(context_task, "Script");
				help_class = s->get_language()->get_global_class_name(context_task);
			}
			if (help_class.is_empty()) {
				// Assuming context task is core class.
				help_class = context_task;
			}
			ScriptEditor::get_singleton()->goto_help("class_name:" + help_class);
			EditorNode::get_singleton()->set_visible_editor(EditorNode::EDITOR_SCRIPT);
		} break;
		case MENU_EDIT_SCRIPT: {
			ERR_FAIL_COND(!context_task.begins_with("res://"));
			ScriptEditor::get_singleton()->open_file(context_task);
		} break;
		case MENU_FAVORITE: {
			Array favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
			if (favorite_tasks.has(context_task)) {
				favorite_tasks.erase(context_task);
			} else {
				favorite_tasks.append(context_task);
			}
			ProjectSettings::get_singleton()->set_setting("limbo_ai/behavior_tree/favorite_tasks", favorite_tasks);
			ProjectSettings::get_singleton()->save();
			emit_signal(SNAME("favorite_tasks_changed"));
		} break;
	}
}

void TaskPanel::_on_task_button_pressed(const String &p_task) {
	emit_signal(SNAME("task_selected"), p_task);
}

void TaskPanel::_on_task_button_rmb(const String &p_task) {
	ERR_FAIL_COND(p_task.is_empty());

	context_task = p_task;
	menu->clear();

	menu->add_icon_item(get_theme_icon(SNAME("Script"), SNAME("EditorIcons")), TTR("Edit Script"), MENU_EDIT_SCRIPT);
	menu->set_item_disabled(MENU_EDIT_SCRIPT, !context_task.begins_with("res://"));
	menu->add_icon_item(get_theme_icon(SNAME("Help"), SNAME("EditorIcons")), TTR("Open Documentation"), MENU_OPEN_DOC);

	menu->add_separator();
	Array favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
	if (favorite_tasks.has(context_task)) {
		menu->add_icon_item(get_theme_icon(SNAME("NonFavorite"), SNAME("EditorIcons")), TTR("Remove from Favorites"), MENU_FAVORITE);
	} else {
		menu->add_icon_item(get_theme_icon(SNAME("Favorites"), SNAME("EditorIcons")), TTR("Add to Favorites"), MENU_FAVORITE);
	}

	menu->reset_size();
	menu->set_position(get_screen_position() + get_local_mouse_position());
	menu->popup();
}

void TaskPanel::_on_filter_text_changed(String p_text) {
	for (int i = 0; i < sections->get_child_count(); i++) {
		TaskSection *sec = Object::cast_to<TaskSection>(sections->get_child(i));
		ERR_FAIL_COND(sec == nullptr);
		sec->set_filter(p_text);
	}
}

void TaskPanel::refresh() {
	filter_edit->set_right_icon(get_theme_icon(SNAME("Search"), SNAME("EditorIcons")));

	HashSet<String> collapsed_sections;
	if (sections->get_child_count() == 0) {
		// Restore collapsed state from config.
		ConfigFile conf;
		String conf_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("limbo_ai.cfg");
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
			sections->get_child(i)->queue_free();
		}
	}

	HashMap<String, List<String>> categorized_tasks;

	categorized_tasks["Composites"] = List<String>();
	_populate_core_tasks_from_class("BTComposite", &categorized_tasks["Composites"]);

	categorized_tasks["Actions"] = List<String>();
	_populate_core_tasks_from_class("BTAction", &categorized_tasks["Actions"]);

	categorized_tasks["Decorators"] = List<String>();
	_populate_core_tasks_from_class("BTDecorator", &categorized_tasks["Decorators"]);

	categorized_tasks["Conditions"] = List<String>();
	_populate_core_tasks_from_class("BTCondition", &categorized_tasks["Conditions"]);

	categorized_tasks["User"] = List<String>();

	String dir1 = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_1");
	_populate_from_user_dir(dir1, &categorized_tasks);

	String dir2 = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_2");
	_populate_from_user_dir(dir2, &categorized_tasks);

	String dir3 = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_3");
	_populate_from_user_dir(dir3, &categorized_tasks);

	List<String> categories;
	for (KeyValue<String, List<String>> &K : categorized_tasks) {
		K.value.sort();
		categories.push_back(K.key);
	}
	categories.sort();
	for (String cat : categories) {
		List<String> tasks = categorized_tasks.get(cat);

		if (tasks.size() == 0) {
			continue;
		}

		TaskSection *sec = memnew(TaskSection(cat));
		for (String task_meta : tasks) {
			Ref<Texture2D> icon = LimboUtility::get_singleton()->get_task_icon(task_meta);
			String tname;
			if (task_meta.begins_with("res:")) {
				tname = task_meta.get_file().get_basename().trim_prefix("BT").to_pascal_case();
			} else {
				tname = task_meta.trim_prefix("BT");
			}
			sec->add_task_button(tname, icon, task_meta);
		}
		sec->set_filter("");
		sec->connect(SNAME("task_button_pressed"), callable_mp(this, &TaskPanel::_on_task_button_pressed));
		sec->connect(SNAME("task_button_rmb"), callable_mp(this, &TaskPanel::_on_task_button_rmb));
		sections->add_child(sec);
		sec->set_collapsed(collapsed_sections.has(cat));
	}
}

void TaskPanel::_populate_core_tasks_from_class(const StringName &p_base_class, List<String> *p_task_classes) {
	List<StringName> inheriters;
	ClassDB::get_inheriters_from_class(p_base_class, &inheriters);

	for (StringName cl : inheriters) {
		p_task_classes->push_back(cl);
	}
}

void TaskPanel::_populate_from_user_dir(String p_path, HashMap<String, List<String>> *p_categories) {
	if (p_path.is_empty()) {
		return;
	}
	Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	if (dir->change_dir(p_path) == OK) {
		dir->list_dir_begin();
		String fn = dir->get_next();
		while (!fn.is_empty()) {
			if (dir->current_is_dir() && fn != "..") {
				String full_path;
				String category;
				if (fn == ".") {
					full_path = p_path;
					category = "User";
				} else {
					full_path = p_path.path_join(fn);
					category = fn.capitalize();
				}

				if (!p_categories->has(category)) {
					p_categories->insert(category, List<String>());
				}

				_populate_scripted_tasks_from_dir(full_path, &p_categories->get(category));
			}
			fn = dir->get_next();
		}
		dir->list_dir_end();
	} else {
		ERR_FAIL_MSG(vformat("Failed to list \"%s\" directory.", p_path));
	}
}

void TaskPanel::_populate_scripted_tasks_from_dir(String p_path, List<String> *p_task_classes) {
	if (p_path.is_empty()) {
		return;
	}
	Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	if (dir->change_dir(p_path) == OK) {
		dir->list_dir_begin();
		String fn = dir->get_next();
		while (!fn.is_empty()) {
			if (fn.ends_with(".gd")) {
				String full_path = p_path.path_join(fn);
				p_task_classes->push_back(full_path);
			}
			fn = dir->get_next();
		}
		dir->list_dir_end();
	} else {
		ERR_FAIL_MSG(vformat("Failed to list \"%s\" directory.", p_path));
	}
}

void TaskPanel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_EXIT_TREE: {
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
			String conf_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("limbo_ai.cfg");
			conf.load(conf_path);
			conf.set_value("bt_editor", "collapsed_sections", collapsed_sections);
			conf.save(conf_path);
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			if (is_visible_in_tree()) {
				refresh();
			}
		} break;
	}
}

void TaskPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("refresh"), &TaskPanel::refresh);

	ADD_SIGNAL(MethodInfo("task_selected"));
	ADD_SIGNAL(MethodInfo("favorite_tasks_changed"));
}

TaskPanel::TaskPanel() {
	VBoxContainer *vb = memnew(VBoxContainer);
	add_child(vb);

	filter_edit = memnew(LineEdit);
	filter_edit->set_clear_button_enabled(true);
	filter_edit->set_placeholder(TTR("Filter tasks"));
	filter_edit->connect("text_changed", callable_mp(this, &TaskPanel::_on_filter_text_changed));
	vb->add_child(filter_edit);

	ScrollContainer *sc = memnew(ScrollContainer);
	sc->set_h_size_flags(SIZE_EXPAND_FILL);
	sc->set_v_size_flags(SIZE_EXPAND_FILL);
	vb->add_child(sc);

	sections = memnew(VBoxContainer);
	sections->set_h_size_flags(SIZE_EXPAND_FILL);
	sections->set_v_size_flags(SIZE_EXPAND_FILL);
	sc->add_child(sections);

	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", callable_mp(this, &TaskPanel::_menu_action_selected));
}

TaskPanel::~TaskPanel() {
}

//**** TaskPanel ^

//**** LimboAIEditor

void LimboAIEditor::_add_task(const Ref<BTTask> &p_task) {
	ERR_FAIL_COND(p_task.is_null());
	ERR_FAIL_COND(task_tree->get_bt().is_null());
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Add BT Task"));
	Ref<BTTask> parent = task_tree->get_selected();
	if (parent.is_null()) {
		parent = task_tree->get_bt()->get_root_task();
	}
	if (parent.is_null()) {
		undo_redo->add_do_method(task_tree->get_bt().ptr(), SNAME("set_root_task"), p_task);
		undo_redo->add_undo_method(task_tree->get_bt().ptr(), SNAME("set_root_task"), task_tree->get_bt()->get_root_task());
	} else {
		undo_redo->add_do_method(parent.ptr(), SNAME("add_child"), p_task);
		undo_redo->add_undo_method(parent.ptr(), SNAME("remove_child"), p_task);
	}
	undo_redo->add_do_method(task_tree, SNAME("update_tree"));
	undo_redo->add_undo_method(task_tree, SNAME("update_tree"));
	undo_redo->commit_action();
	_mark_as_dirty(true);
}

void LimboAIEditor::_add_task_by_class_or_path(String p_class_or_path) {
	Ref<BTTask> task;

	if (p_class_or_path.begins_with("res:")) {
		Ref<Script> s = ResourceLoader::load(p_class_or_path, "Script");
		ERR_FAIL_COND_MSG(s.is_null() || !s->is_valid(), vformat("LimboAI: Failed to instantiate task. Bad script: %s", p_class_or_path));
		Variant inst = ClassDB::instantiate(s->get_instance_base_type());
		ERR_FAIL_COND_MSG(inst.is_zero(), vformat("LimboAI: Failed to instantiate base type \"%s\".", s->get_instance_base_type()));

		if (unlikely(!((Object *)inst)->is_class("BTTask"))) {
			if (!inst.is_ref_counted()) {
				memdelete((Object *)inst);
			}
			ERR_PRINT(vformat("LimboAI: Failed to instantiate task. Script is not a BTTask: %s", p_class_or_path));
			return;
		}

		if (inst && s.is_valid()) {
			((Object *)inst)->set_script(s);
			task = inst;
		}
	} else {
		task = ClassDB::instantiate(p_class_or_path);
	}
	_add_task(task);
}

void LimboAIEditor::_remove_task(const Ref<BTTask> &p_task) {
	ERR_FAIL_COND(p_task.is_null());
	ERR_FAIL_COND(task_tree->get_bt().is_null());
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Remove BT Task"));
	if (p_task->get_parent() == nullptr) {
		ERR_FAIL_COND(task_tree->get_bt()->get_root_task() != p_task);
		undo_redo->add_do_method(task_tree->get_bt().ptr(), SNAME("set_root_task"), Variant());
		undo_redo->add_undo_method(task_tree->get_bt().ptr(), SNAME("set_root_task"), task_tree->get_bt()->get_root_task());
	} else {
		undo_redo->add_do_method(p_task->get_parent().ptr(), SNAME("remove_child"), p_task);
		undo_redo->add_undo_method(p_task->get_parent().ptr(), SNAME("add_child_at_index"), p_task, p_task->get_parent()->get_child_index(p_task));
	}
	undo_redo->add_do_method(task_tree, SNAME("update_tree"));
	undo_redo->add_undo_method(task_tree, SNAME("update_tree"));
	undo_redo->commit_action();
}

void LimboAIEditor::_update_header() const {
	if (task_tree->get_bt().is_null()) {
		header->set_text("");
		header->set_icon(nullptr);
		return;
	}

	String text = task_tree->get_bt()->get_path();
	if (text.is_empty()) {
		text = TTR("New Behavior Tree");
	} else if (dirty.has(task_tree->get_bt())) {
		text += "(*)";
	}

	header->set_text(text);
	header->set_icon(EditorNode::get_singleton()->get_object_icon(task_tree->get_bt().ptr(), "BehaviorTree"));
}

void LimboAIEditor::_update_history_buttons() {
	history_back->set_disabled(idx_history == 0);
	history_forward->set_disabled(idx_history == (history.size() - 1));
}

void LimboAIEditor::_new_bt() {
	BehaviorTree *bt = memnew(BehaviorTree);
	bt->set_root_task(memnew(BTSelector));
	EditorNode::get_singleton()->edit_resource(bt);
}

void LimboAIEditor::_save_bt(String p_path) {
	ERR_FAIL_COND_MSG(p_path.is_empty(), "Empty p_path");
	ERR_FAIL_COND_MSG(task_tree->get_bt().is_null(), "Behavior Tree is null.");
	task_tree->get_bt()->set_path(p_path, true);
	ResourceSaver::save(task_tree->get_bt(), p_path, ResourceSaver::FLAG_CHANGE_PATH);
	_update_header();
	_mark_as_dirty(false);
}

void LimboAIEditor::_load_bt(String p_path) {
	ERR_FAIL_COND_MSG(p_path.is_empty(), "Empty p_path");
	Ref<BehaviorTree> bt = ResourceLoader::load(p_path, "BehaviorTree");
	ERR_FAIL_COND(!bt.is_valid());

	if (history.find(bt) != -1) {
		history.erase(bt);
		history.push_back(bt);
	}

	EditorNode::get_singleton()->edit_resource(bt);
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

void LimboAIEditor::shortcut_input(const Ref<InputEvent> &p_event) {
	if (!p_event->is_pressed()) {
		return;
	}

	// * Global shortcuts.

	if (ED_IS_SHORTCUT("limbo_ai/open_debugger", p_event)) {
		_misc_option_selected(MISC_OPEN_DEBUGGER);
		accept_event();
	}

	// * Local shortcuts.

	if (!(has_focus() || is_ancestor_of(get_viewport()->gui_get_focus_owner()))) {
		return;
	}

	if (ED_IS_SHORTCUT("limbo_ai/rename_task", p_event)) {
		_action_selected(ACTION_RENAME);
	} else if (ED_IS_SHORTCUT("limbo_ai/move_task_up", p_event)) {
		_action_selected(ACTION_MOVE_UP);
	} else if (ED_IS_SHORTCUT("limbo_ai/move_task_down", p_event)) {
		_action_selected(ACTION_MOVE_DOWN);
	} else if (ED_IS_SHORTCUT("limbo_ai/duplicate_task", p_event)) {
		_action_selected(ACTION_DUPLICATE);
	} else if (ED_IS_SHORTCUT("limbo_ai/remove_task", p_event)) {
		_action_selected(ACTION_REMOVE);
	} else if (ED_IS_SHORTCUT("limbo_ai/new_behavior_tree", p_event)) {
		_new_bt();
	} else if (ED_IS_SHORTCUT("limbo_ai/save_behavior_tree", p_event)) {
		_on_save_pressed();
	} else if (ED_IS_SHORTCUT("limbo_ai/load_behavior_tree", p_event)) {
		load_dialog->popup_file_dialog();
	} else {
		return;
	}

	accept_event();
}

void LimboAIEditor::_on_tree_rmb(const Vector2 &p_menu_pos) {
	menu->clear();

	Ref<BTTask> task = task_tree->get_selected();
	ERR_FAIL_COND_MSG(task.is_null(), "LimboAIEditor: get_selected() returned null");

	menu->add_icon_shortcut(get_theme_icon(SNAME("Rename"), SNAME("EditorIcons")), ED_GET_SHORTCUT("limbo_ai/rename_task"), ACTION_RENAME);
	menu->add_icon_item(get_theme_icon(SNAME("Script"), SNAME("EditorIcons")), TTR("Edit Script"), ACTION_EDIT_SCRIPT);
	menu->add_icon_item(get_theme_icon(SNAME("Help"), SNAME("EditorIcons")), TTR("Open Documentation"), ACTION_OPEN_DOC);
	menu->set_item_disabled(ACTION_EDIT_SCRIPT, task->get_script().is_null());

	menu->add_separator();
	menu->add_icon_shortcut(get_theme_icon(SNAME("MoveUp"), SNAME("EditorIcons")), ED_GET_SHORTCUT("limbo_ai/move_task_up"), ACTION_MOVE_UP);
	menu->add_icon_shortcut(get_theme_icon(SNAME("MoveDown"), SNAME("EditorIcons")), ED_GET_SHORTCUT("limbo_ai/move_task_down"), ACTION_MOVE_DOWN);
	menu->add_icon_shortcut(get_theme_icon(SNAME("Duplicate"), SNAME("EditorIcons")), ED_GET_SHORTCUT("limbo_ai/duplicate_task"), ACTION_DUPLICATE);
	menu->add_icon_item(get_theme_icon(SNAME("NewRoot"), SNAME("EditorIcons")), TTR("Make Root"), ACTION_MAKE_ROOT);

	menu->add_separator();
	menu->add_icon_shortcut(get_theme_icon(SNAME("Remove"), SNAME("EditorIcons")), ED_GET_SHORTCUT("limbo_ai/remove_task"), ACTION_REMOVE);

	menu->reset_size();
	menu->set_position(p_menu_pos);
	menu->popup();
}

void LimboAIEditor::_action_selected(int p_id) {
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	switch (p_id) {
		case ACTION_RENAME: {
			if (!task_tree->get_selected().is_valid()) {
				return;
			}
			Ref<BTTask> task = task_tree->get_selected();
			if (task->is_class_ptr(BTComment::get_class_ptr_static())) {
				rename_dialog->set_title(TTR("Edit Comment"));
				rename_dialog->get_ok_button()->set_text(TTR("OK"));
				rename_edit->set_placeholder(TTR("Comment"));
			} else {
				rename_dialog->set_title(TTR("Rename Task"));
				rename_dialog->get_ok_button()->set_text(TTR("Rename"));
				rename_edit->set_placeholder(TTR("Custom Name"));
			}
			rename_edit->set_text(task->get_custom_name());
			rename_dialog->popup_centered();
			rename_edit->select_all();
			rename_edit->grab_focus();
		} break;
		case ACTION_EDIT_SCRIPT: {
			ERR_FAIL_COND(task_tree->get_selected().is_null());
			EditorNode::get_singleton()->edit_resource(task_tree->get_selected()->get_script());
		} break;
		case ACTION_OPEN_DOC: {
			Ref<BTTask> task = task_tree->get_selected();
			ERR_FAIL_COND(task.is_null());
			String help_class;
			if (!task->get_script().is_null()) {
				Ref<Script> s = task->get_script();
				help_class = s->get_language()->get_global_class_name(s->get_path());
			}
			if (help_class.is_empty()) {
				help_class = task->get_class();
			}
			ScriptEditor::get_singleton()->goto_help("class_name:" + help_class);
			EditorNode::get_singleton()->set_visible_editor(EditorNode::EDITOR_SCRIPT);
		} break;
		case ACTION_MOVE_UP: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid() && sel->get_parent().is_valid()) {
				Ref<BTTask> parent = sel->get_parent();
				int idx = parent->get_child_index(sel);
				if (idx > 0 && idx < parent->get_child_count()) {
					undo_redo->create_action(TTR("Move BT Task"));
					undo_redo->add_do_method(parent.ptr(), SNAME("remove_child"), sel);
					undo_redo->add_do_method(parent.ptr(), SNAME("add_child_at_index"), sel, idx - 1);
					undo_redo->add_undo_method(parent.ptr(), SNAME("remove_child"), sel);
					undo_redo->add_undo_method(parent.ptr(), SNAME("add_child_at_index"), sel, idx);
					undo_redo->add_do_method(task_tree, SNAME("update_tree"));
					undo_redo->add_undo_method(task_tree, SNAME("update_tree"));
					undo_redo->commit_action();
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
					undo_redo->create_action(TTR("Move BT Task"));
					undo_redo->add_do_method(parent.ptr(), SNAME("remove_child"), sel);
					undo_redo->add_do_method(parent.ptr(), SNAME("add_child_at_index"), sel, idx + 1);
					undo_redo->add_undo_method(parent.ptr(), SNAME("remove_child"), sel);
					undo_redo->add_undo_method(parent.ptr(), SNAME("add_child_at_index"), sel, idx);
					undo_redo->add_do_method(task_tree, SNAME("update_tree"));
					undo_redo->add_undo_method(task_tree, SNAME("update_tree"));
					undo_redo->commit_action();
					_mark_as_dirty(true);
				}
			}
		} break;
		case ACTION_DUPLICATE: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid()) {
				undo_redo->create_action(TTR("Duplicate BT Task"));
				Ref<BTTask> parent = sel->get_parent();
				if (parent.is_null()) {
					parent = sel;
				}
				const Ref<BTTask> &sel_dup = sel->clone();
				undo_redo->add_do_method(parent.ptr(), SNAME("add_child_at_index"), sel_dup, parent->get_child_index(sel) + 1);
				undo_redo->add_undo_method(parent.ptr(), SNAME("remove_child"), sel_dup);
				undo_redo->add_do_method(task_tree, SNAME("update_tree"));
				undo_redo->add_undo_method(task_tree, SNAME("update_tree"));
				undo_redo->commit_action();
				_mark_as_dirty(true);
			}
		} break;
		case ACTION_MAKE_ROOT: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid() && task_tree->get_bt()->get_root_task() != sel) {
				Ref<BTTask> parent = sel->get_parent();
				ERR_FAIL_COND(parent.is_null());
				undo_redo->create_action(TTR("Make Root"));
				undo_redo->add_do_method(parent.ptr(), SNAME("remove_child"), sel);
				Ref<BTTask> old_root = task_tree->get_bt()->get_root_task();
				undo_redo->add_do_method(task_tree->get_bt().ptr(), SNAME("set_root_task"), sel);
				undo_redo->add_do_method(sel.ptr(), SNAME("add_child"), old_root);
				undo_redo->add_undo_method(sel.ptr(), SNAME("remove_child"), old_root);
				undo_redo->add_undo_method(task_tree->get_bt().ptr(), SNAME("set_root_task"), old_root);
				undo_redo->add_undo_method(parent.ptr(), SNAME("add_child_at_index"), sel, parent->get_child_index(sel));
				undo_redo->add_do_method(task_tree, SNAME("update_tree"));
				undo_redo->add_undo_method(task_tree, SNAME("update_tree"));
				undo_redo->commit_action();
				_mark_as_dirty(true);
			}
		} break;
		case ACTION_REMOVE: {
			Ref<BTTask> sel = task_tree->get_selected();
			if (sel.is_valid()) {
				undo_redo->create_action(TTR("Remove BT Task"));
				if (sel->get_parent().is_null()) {
					undo_redo->add_do_method(task_tree->get_bt().ptr(), SNAME("set_root_task"), Variant());
					undo_redo->add_undo_method(task_tree->get_bt().ptr(), SNAME("set_root_task"), task_tree->get_bt()->get_root_task());
				} else {
					undo_redo->add_do_method(sel->get_parent().ptr(), SNAME("remove_child"), sel);
					undo_redo->add_undo_method(sel->get_parent().ptr(), SNAME("add_child_at_index"), sel, sel->get_parent()->get_child_index(sel));
				}
				undo_redo->add_do_method(task_tree, SNAME("update_tree"));
				undo_redo->add_undo_method(task_tree, SNAME("update_tree"));
				undo_redo->commit_action();
				EditorNode::get_singleton()->edit_resource(task_tree->get_selected());
				_mark_as_dirty(true);
			}
		} break;
	}
}

void LimboAIEditor::_misc_option_selected(int p_id) {
	switch (p_id) {
		case MISC_OPEN_DEBUGGER: {
			ERR_FAIL_COND(LimboDebuggerPlugin::get_singleton() == nullptr);
			if (LimboDebuggerPlugin::get_singleton()->get_session_tab()->get_window_enabled()) {
				LimboDebuggerPlugin::get_singleton()->get_session_tab()->set_window_enabled(true);
			} else {
				EditorNode::get_singleton()->make_bottom_panel_item_visible(EditorDebuggerNode::get_singleton());
				EditorDebuggerNode::get_singleton()->get_default_debugger()->switch_to_debugger(
						LimboDebuggerPlugin::get_singleton()->get_session_tab_index());
			}
		} break;
		case MISC_PROJECT_SETTINGS: {
			ProjectSettingsEditor::get_singleton()->set_general_page("limbo_ai/behavior_tree");
			ProjectSettingsEditor::get_singleton()->popup_project_settings();
		} break;
		case MISC_CREATE_SCRIPT_TEMPLATE: {
			if (!FileAccess::exists("res://script_templates/BTTask/custom_task.gd")) {
				Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_RESOURCES);
				if (!dir->exists("res://script_templates/")) {
					dir->make_dir("res://script_templates/");
				}
				if (!dir->exists("res://script_templates/BTTask")) {
					dir->make_dir("res://script_templates/BTTask");
				}
				Error err;
				Ref<FileAccess> f = FileAccess::open("res://script_templates/BTTask/custom_task.gd", FileAccess::WRITE, &err);
				ERR_FAIL_COND(err != OK);

				String script_template =
						"# meta-name: Custom Task\n"
						"# meta-description: Custom task to be used in a BehaviorTree\n"
						"# meta-default: true\n"
						"@tool\n"
						"extends _BASE_\n"
						"## _CLASS_\n"
						"\n\n"
						"# Display a customized name (requires @tool).\n"
						"func _generate_name() -> String:\n"
						"_TS_return \"_CLASS_\"\n"
						"\n\n"
						"# Called once during initialization.\n"
						"func _setup() -> void:\n"
						"_TS_pass\n"
						"\n\n"
						"# Called each time this task is entered.\n"
						"func _enter() -> void:\n"
						"_TS_pass\n"
						"\n\n"
						"# Called each time this task is exited.\n"
						"func _exit() -> void:\n"
						"_TS_pass\n"
						"\n\n"
						"# Called each time this task is ticked (aka executed).\n"
						"func _tick(delta: float) -> int:\n"
						"_TS_return SUCCESS\n";

				f->store_string(script_template);
				f->close();
			}

			ScriptEditor::get_singleton()->open_file("res://script_templates/BTTask/custom_task.gd");

		} break;
	}
}

void LimboAIEditor::_on_tree_task_selected(const Ref<BTTask> &p_task) {
	EditorNode::get_singleton()->edit_resource(p_task);
}

void LimboAIEditor::_on_tree_task_double_clicked() {
	_action_selected(ACTION_RENAME);
}

void LimboAIEditor::_on_visibility_changed() {
	if (task_tree->is_visible()) {
		Ref<BTTask> sel = task_tree->get_selected();
		if (sel.is_valid()) {
			EditorNode::get_singleton()->edit_resource(sel);
		} else if (task_tree->get_bt().is_valid() && InspectorDock::get_inspector_singleton()->get_edited_object() != task_tree->get_bt().ptr()) {
			EditorNode::get_singleton()->edit_resource(task_tree->get_bt());
		}

		task_panel->refresh();
	}
	_update_favorite_tasks();
}

void LimboAIEditor::_on_header_pressed() {
	_update_header();
	task_tree->deselect();
	EditorNode::get_singleton()->edit_resource(task_tree->get_bt());
}

void LimboAIEditor::_on_save_pressed() {
	if (task_tree->get_bt().is_null()) {
		return;
	}
	String path = task_tree->get_bt()->get_path();
	if (path.is_empty()) {
		save_dialog->popup_centered_ratio();
	} else {
		_save_bt(path);
	}
}

void LimboAIEditor::_on_history_back() {
	idx_history = MAX(idx_history - 1, 0);
	EditorNode::get_singleton()->edit_resource(history[idx_history]);
}

void LimboAIEditor::_on_history_forward() {
	idx_history = MIN(idx_history + 1, history.size() - 1);
	EditorNode::get_singleton()->edit_resource(history[idx_history]);
}

void LimboAIEditor::_on_task_dragged(Ref<BTTask> p_task, Ref<BTTask> p_to_task, int p_type) {
	ERR_FAIL_COND(p_type < -1 || p_type > 1);
	if (p_task == p_to_task) {
		return;
	}

	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Drag BT Task"));
	undo_redo->add_do_method(p_task->get_parent().ptr(), SNAME("remove_child"), p_task);

	if (p_type == 0) {
		undo_redo->add_do_method(p_to_task.ptr(), SNAME("add_child"), p_task);
		undo_redo->add_undo_method(p_to_task.ptr(), SNAME("remove_child"), p_task);
	} else if (p_type == -1 && p_to_task->get_parent().is_valid()) {
		undo_redo->add_do_method(p_to_task->get_parent().ptr(), SNAME("add_child_at_index"), p_task, p_to_task->get_parent()->get_child_index(p_to_task));
		undo_redo->add_undo_method(p_to_task->get_parent().ptr(), SNAME("remove_child"), p_task);
	} else if (p_type == 1 && p_to_task->get_parent().is_valid()) {
		undo_redo->add_do_method(p_to_task->get_parent().ptr(), SNAME("add_child_at_index"), p_task, p_to_task->get_parent()->get_child_index(p_to_task) + 1);
		undo_redo->add_undo_method(p_to_task->get_parent().ptr(), SNAME("remove_child"), p_task);
	}

	undo_redo->add_undo_method(p_task->get_parent().ptr(), "add_child_at_index", p_task, p_task->get_parent()->get_child_index(p_task));

	undo_redo->add_do_method(task_tree, SNAME("update_tree"));
	undo_redo->add_undo_method(task_tree, SNAME("update_tree"));

	undo_redo->commit_action();
	_mark_as_dirty(true);
}

void LimboAIEditor::_on_resources_reload(const Vector<String> &p_resources) {
	for (const String &res_path : p_resources) {
		if (!ResourceCache::has(res_path)) {
			continue;
		}

		String res_type = ResourceLoader::get_resource_type(res_path);
		if (res_type == "BehaviorTree") {
			Ref<Resource> res = ResourceCache::get_ref(res_path);
			if (res.is_valid()) {
				if (history.has(res)) {
					disk_changed_files.insert(res_path);
				} else {
					res->reload_from_file();
				}
			}
		}
	}

	if (disk_changed_files.size() > 0) {
		disk_changed_list->clear();
		disk_changed_list->set_hide_root(true);
		disk_changed_list->create_item();
		for (const String &fn : disk_changed_files) {
			TreeItem *ti = disk_changed_list->create_item();
			ti->set_text(0, fn);
		}

		if (!is_visible()) {
			EditorNode::get_singleton()->select_editor_by_name("LimboAI");
		}
		disk_changed->call_deferred("popup_centered_ratio", 0.5);
	}
}

void LimboAIEditor::_reload_modified() {
	for (const String &fn : disk_changed_files) {
		Ref<Resource> res = ResourceCache::get_ref(fn);
		if (res.is_valid()) {
			ERR_FAIL_COND(!res->is_class("BehaviorTree"));
			res->reload_from_file();
			if (idx_history >= 0 && history.get(idx_history) == res) {
				edit_bt(res, true);
			}
		}
	}
	disk_changed_files.clear();
}

void LimboAIEditor::_resave_modified(String _str) {
	for (const String &fn : disk_changed_files) {
		Ref<Resource> res = ResourceCache::get_ref(fn);
		if (res.is_valid()) {
			ERR_FAIL_COND(!res->is_class("BehaviorTree"));
			ResourceSaver::save(res, res->get_path());
		}
	}
	disk_changed->hide();
	disk_changed_files.clear();
}

void LimboAIEditor::_rename_task_confirmed() {
	ERR_FAIL_COND(!task_tree->get_selected().is_valid());
	rename_dialog->hide();

	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Set Custom Name"));
	undo_redo->add_do_method(task_tree->get_selected().ptr(), SNAME("set_custom_name"), rename_edit->get_text());
	undo_redo->add_undo_method(task_tree->get_selected().ptr(), SNAME("set_custom_name"), task_tree->get_selected()->get_custom_name());
	undo_redo->add_do_method(task_tree, SNAME("update_task"), task_tree->get_selected());
	undo_redo->add_undo_method(task_tree, SNAME("update_task"), task_tree->get_selected());
	undo_redo->commit_action();
}

void LimboAIEditor::apply_changes() {
	for (int i = 0; i < history.size(); i++) {
		Ref<BehaviorTree> bt = history.get(i);
		String path = bt->get_path();
		if (ResourceLoader::exists(path)) {
			ResourceSaver::save(bt, path);
		}
		dirty.clear();
		_update_header();
	}
}

void LimboAIEditor::_update_favorite_tasks() {
	for (int i = 0; i < fav_tasks_hbox->get_child_count(); i++) {
		fav_tasks_hbox->get_child(i)->queue_free();
	}
	Array favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
	for (int i = 0; i < favorite_tasks.size(); i++) {
		String task_meta = favorite_tasks[i];
		Button *btn = memnew(Button);
		String task_name;
		if (task_meta.begins_with("res:")) {
			task_name = task_meta.get_file().get_basename().trim_prefix("BT").to_pascal_case();
		} else {
			task_name = task_meta.trim_prefix("BT");
		}
		btn->set_text(task_name);
		btn->set_meta(SNAME("task_meta"), task_meta);
		btn->set_icon(LimboUtility::get_singleton()->get_task_icon(task_meta));
		btn->set_tooltip_text(vformat(TTR("Add %s task."), task_name));
		btn->set_flat(true);
		btn->add_theme_constant_override(SNAME("icon_max_width"), 16 * EDSCALE); // Force user icons to be of the proper size.
		btn->set_focus_mode(Control::FOCUS_NONE);
		btn->connect(SNAME("pressed"), callable_mp(this, &LimboAIEditor::_add_task_by_class_or_path).bind(task_meta));
		fav_tasks_hbox->add_child(btn);
	}
}

void LimboAIEditor::_update_misc_menu() {
	PopupMenu *misc_menu = misc_btn->get_popup();

	misc_menu->clear();

	misc_menu->add_icon_shortcut(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Debug"), SNAME("EditorIcons")), ED_GET_SHORTCUT("limbo_ai/open_debugger"), MISC_OPEN_DEBUGGER);
	misc_menu->add_item(TTR("Project Settings..."), MISC_PROJECT_SETTINGS);

	misc_menu->add_separator();
	misc_menu->add_item(
			FileAccess::exists("res://script_templates/BTAction/custom_action.gd") ? TTR("Edit Script Template") : TTR("Create Script Template"),
			MISC_CREATE_SCRIPT_TEMPLATE);
}

void LimboAIEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			ConfigFile conf;
			String conf_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("limbo_ai.cfg");
			if (conf.load(conf_path) == OK) {
				hsc->set_split_offset(conf.get_value("bt_editor", "bteditor_hsplit", hsc->get_split_offset()));
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			ConfigFile conf;
			String conf_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("limbo_ai.cfg");
			conf.load(conf_path);
			conf.set_value("bt_editor", "bteditor_hsplit", hsc->get_split_offset());
			conf.save(conf_path);
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			new_btn->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("New"), SNAME("EditorIcons")));
			load_btn->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Load"), SNAME("EditorIcons")));
			save_btn->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Save"), SNAME("EditorIcons")));
			new_script_btn->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("ScriptCreate"), SNAME("EditorIcons")));
			history_back->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Back"), SNAME("EditorIcons")));
			history_forward->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Forward"), SNAME("EditorIcons")));

			misc_btn->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Tools"), SNAME("EditorIcons")));

			_update_favorite_tasks();
			_update_header();
		}
	}
}

void LimboAIEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_add_task", "p_task"), &LimboAIEditor::_add_task);
	ClassDB::bind_method(D_METHOD("_remove_task", "p_task"), &LimboAIEditor::_remove_task);
	ClassDB::bind_method(D_METHOD("_add_task_with_prototype", "p_prototype"), &LimboAIEditor::_add_task_with_prototype);
	ClassDB::bind_method(D_METHOD("_new_bt"), &LimboAIEditor::_new_bt);
	ClassDB::bind_method(D_METHOD("_save_bt", "p_path"), &LimboAIEditor::_save_bt);
	ClassDB::bind_method(D_METHOD("_load_bt", "p_path"), &LimboAIEditor::_load_bt);
	ClassDB::bind_method(D_METHOD("edit_bt", "p_behavior_tree", "p_force_refresh"), &LimboAIEditor::edit_bt, Variant(false));
	ClassDB::bind_method(D_METHOD("_reload_modified"), &LimboAIEditor::_reload_modified);
	ClassDB::bind_method(D_METHOD("_resave_modified"), &LimboAIEditor::_resave_modified);
}

LimboAIEditor::LimboAIEditor() {
	idx_history = 0;

	ED_SHORTCUT("limbo_ai/rename_task", TTR("Rename"), Key::F2);
	ED_SHORTCUT_OVERRIDE("limbo_ai/rename_task", "macos", Key::ENTER);
	ED_SHORTCUT("limbo_ai/move_task_up", TTR("Move Up"), KeyModifierMask::CMD_OR_CTRL | Key::UP);
	ED_SHORTCUT("limbo_ai/move_task_down", TTR("Move Down"), KeyModifierMask::CMD_OR_CTRL | Key::DOWN);
	ED_SHORTCUT("limbo_ai/duplicate_task", TTR("Duplicate"), KeyModifierMask::CMD_OR_CTRL | Key::D);
	ED_SHORTCUT("limbo_ai/remove_task", TTR("Remove"), Key::KEY_DELETE);

	ED_SHORTCUT("limbo_ai/new_behavior_tree", TTR("New Behavior Tree"), KeyModifierMask::CMD_OR_CTRL | KeyModifierMask::ALT | Key::N);
	ED_SHORTCUT("limbo_ai/save_behavior_tree", TTR("Save Behavior Tree"), KeyModifierMask::CMD_OR_CTRL | KeyModifierMask::ALT | Key::S);
	ED_SHORTCUT("limbo_ai/load_behavior_tree", TTR("Load Behavior Tree"), KeyModifierMask::CMD_OR_CTRL | KeyModifierMask::ALT | Key::L);
	ED_SHORTCUT("limbo_ai/open_debugger", TTR("Open Debugger"), KeyModifierMask::CMD_OR_CTRL | KeyModifierMask::ALT | Key::D);

	set_process_shortcut_input(true);

	save_dialog = memnew(FileDialog);
	save_dialog->set_file_mode(FileDialog::FILE_MODE_SAVE_FILE);
	save_dialog->set_title("Save Behavior Tree");
	save_dialog->add_filter("*.tres");
	save_dialog->connect("file_selected", callable_mp(this, &LimboAIEditor::_save_bt));
	save_dialog->hide();
	add_child(save_dialog);

	load_dialog = memnew(FileDialog);
	load_dialog->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
	load_dialog->set_title("Load Behavior Tree");
	load_dialog->add_filter("*.tres");
	load_dialog->connect("file_selected", callable_mp(this, &LimboAIEditor::_load_bt));
	load_dialog->hide();
	add_child(load_dialog);

	VBoxContainer *vb = memnew(VBoxContainer);
	vb->set_anchor(SIDE_RIGHT, ANCHOR_END);
	vb->set_anchor(SIDE_BOTTOM, ANCHOR_END);
	add_child(vb);

	HBoxContainer *toolbar = memnew(HBoxContainer);
	vb->add_child(toolbar);

	Array favorite_tasks_default;
	favorite_tasks_default.append("BTSelector");
	favorite_tasks_default.append("BTSequence");
	favorite_tasks_default.append("BTParallel");
	GLOBAL_DEF(PropertyInfo(Variant::ARRAY, "limbo_ai/behavior_tree/favorite_tasks", PROPERTY_HINT_ARRAY_TYPE, "String"), favorite_tasks_default);

	fav_tasks_hbox = memnew(HBoxContainer);
	toolbar->add_child(fav_tasks_hbox);

	comment_btn = memnew(Button);
	comment_btn->set_text(TTR("Comment"));
	comment_btn->set_icon(LimboUtility::get_singleton()->get_task_icon("BTComment"));
	comment_btn->set_tooltip_text(TTR("Add a BTComment task."));
	comment_btn->set_flat(true);
	comment_btn->set_focus_mode(Control::FOCUS_NONE);
	comment_btn->connect("pressed", callable_mp(this, &LimboAIEditor::_add_task_by_class_or_path).bind("BTComment"));
	toolbar->add_child(comment_btn);

	toolbar->add_child(memnew(VSeparator));

	new_btn = memnew(Button);
	new_btn->set_text(TTR("New"));
	new_btn->set_tooltip_text(TTR("Create a new behavior tree."));
	new_btn->set_shortcut(ED_GET_SHORTCUT("limbo_ai/new_behavior_tree"));
	new_btn->set_flat(true);
	new_btn->set_focus_mode(Control::FOCUS_NONE);
	new_btn->connect("pressed", callable_mp(this, &LimboAIEditor::_new_bt));
	toolbar->add_child(new_btn);

	load_btn = memnew(Button);
	load_btn->set_text(TTR("Load"));
	load_btn->set_tooltip_text(TTR("Load behavior tree from a resource file."));
	load_btn->set_shortcut(ED_GET_SHORTCUT("limbo_ai/load_behavior_tree"));
	load_btn->set_flat(true);
	load_btn->set_focus_mode(Control::FOCUS_NONE);
	load_btn->connect("pressed", callable_mp(load_dialog, &FileDialog::popup_file_dialog));
	toolbar->add_child(load_btn);

	save_btn = memnew(Button);
	save_btn->set_text(TTR("Save"));
	save_btn->set_tooltip_text(TTR("Save edited behavior tree to a resource file."));
	save_btn->set_shortcut(ED_GET_SHORTCUT("limbo_ai/save_behavior_tree"));
	save_btn->set_flat(true);
	save_btn->set_focus_mode(Control::FOCUS_NONE);
	save_btn->connect("pressed", callable_mp(this, &LimboAIEditor::_on_save_pressed));
	toolbar->add_child(save_btn);

	toolbar->add_child(memnew(VSeparator));

	new_script_btn = memnew(Button);
	new_script_btn->set_text(TTR("New Task"));
	new_script_btn->set_tooltip_text(TTR("Create new task script and edit it."));
	new_script_btn->set_flat(true);
	new_script_btn->set_focus_mode(Control::FOCUS_NONE);
	toolbar->add_child(new_script_btn);

	// toolbar->add_child(memnew(VSeparator));

	misc_btn = memnew(MenuButton);
	misc_btn->set_text(TTR("Misc"));
	misc_btn->set_flat(true);
	misc_btn->connect("pressed", callable_mp(this, &LimboAIEditor::_update_misc_menu));
	misc_btn->get_popup()->connect("id_pressed", callable_mp(this, &LimboAIEditor::_misc_option_selected));
	toolbar->add_child(misc_btn);

	// toolbar->add_child(memnew(VSeparator));

	HBoxContainer *nav = memnew(HBoxContainer);
	nav->set_h_size_flags(SIZE_EXPAND | SIZE_SHRINK_END);
	toolbar->add_child(nav);

	history_back = memnew(Button);
	history_back->set_flat(true);
	history_back->set_focus_mode(FOCUS_NONE);
	history_back->connect("pressed", callable_mp(this, &LimboAIEditor::_on_history_back));
	nav->add_child(history_back);

	history_forward = memnew(Button);
	history_forward->set_flat(true);
	history_forward->set_focus_mode(FOCUS_NONE);
	history_forward->connect("pressed", callable_mp(this, &LimboAIEditor::_on_history_forward));
	nav->add_child(history_forward);

	header = memnew(Button);
	header->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	header->add_theme_constant_override("hseparation", 8);
	header->connect("pressed", callable_mp(this, &LimboAIEditor::_on_header_pressed));
	vb->add_child(header);

	hsc = memnew(HSplitContainer);
	hsc->set_h_size_flags(SIZE_EXPAND_FILL);
	hsc->set_v_size_flags(SIZE_EXPAND_FILL);
	hsc->set_focus_mode(FOCUS_NONE);
	vb->add_child(hsc);

	task_tree = memnew(TaskTree);
	task_tree->set_v_size_flags(SIZE_EXPAND_FILL);
	task_tree->set_h_size_flags(SIZE_EXPAND_FILL);
	task_tree->connect("rmb_pressed", callable_mp(this, &LimboAIEditor::_on_tree_rmb));
	task_tree->connect("task_selected", callable_mp(this, &LimboAIEditor::_on_tree_task_selected));
	task_tree->connect("visibility_changed", callable_mp(this, &LimboAIEditor::_on_visibility_changed));
	task_tree->connect("task_dragged", callable_mp(this, &LimboAIEditor::_on_task_dragged));
	task_tree->connect("task_double_clicked", callable_mp(this, &LimboAIEditor::_on_tree_task_double_clicked));
	task_tree->hide();
	hsc->add_child(task_tree);

	usage_hint = memnew(Panel);
	usage_hint->set_v_size_flags(SIZE_EXPAND_FILL);
	usage_hint->set_h_size_flags(SIZE_EXPAND_FILL);
	hsc->add_child(usage_hint);

	Label *usage_label = memnew(Label);
	usage_label->set_anchor(SIDE_RIGHT, 1);
	usage_label->set_anchor(SIDE_BOTTOM, 1);
	usage_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	usage_label->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	usage_label->set_text(TTR("Create a new or load an existing behavior tree."));
	usage_hint->add_child(usage_label);

	task_panel = memnew(TaskPanel());
	hsc->set_split_offset(-300);
	task_panel->connect("task_selected", callable_mp(this, &LimboAIEditor::_add_task_by_class_or_path));
	task_panel->connect("favorite_tasks_changed", callable_mp(this, &LimboAIEditor::_update_favorite_tasks));
	task_panel->hide();
	hsc->add_child(task_panel);

	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", callable_mp(this, &LimboAIEditor::_action_selected));

	rename_dialog = memnew(ConfirmationDialog);
	{
		VBoxContainer *vbc = memnew(VBoxContainer);
		rename_dialog->add_child(vbc);

		rename_edit = memnew(LineEdit);
		vbc->add_child(rename_edit);
		rename_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		rename_edit->set_custom_minimum_size(Size2(350.0, 0.0));

		rename_dialog->register_text_enter(rename_edit);
		rename_dialog->connect("confirmed", callable_mp(this, &LimboAIEditor::_rename_task_confirmed));
	}
	add_child(rename_dialog);

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

		disk_changed->get_ok_button()->set_text(TTR("Reload"));
		disk_changed->connect("confirmed", callable_mp(this, &LimboAIEditor::_reload_modified));

		disk_changed->add_button(TTR("Resave"), !DisplayServer::get_singleton()->get_swap_cancel_ok(), "resave");
		disk_changed->connect("custom_action", callable_mp(this, &LimboAIEditor::_resave_modified));
	}
	EditorNode::get_singleton()->get_gui_base()->add_child(disk_changed);

	GLOBAL_DEF(PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/behavior_tree_default_dir", PROPERTY_HINT_DIR), "res://ai/trees");
	GLOBAL_DEF(PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/user_task_dir_1", PROPERTY_HINT_DIR), "res://ai/tasks");
	GLOBAL_DEF(PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/user_task_dir_2", PROPERTY_HINT_DIR), "");
	GLOBAL_DEF(PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/user_task_dir_3", PROPERTY_HINT_DIR), "");

	save_dialog->set_current_dir(GLOBAL_GET("limbo_ai/behavior_tree/behavior_tree_default_dir"));
	load_dialog->set_current_dir(GLOBAL_GET("limbo_ai/behavior_tree/behavior_tree_default_dir"));
	new_script_btn->connect("pressed", callable_mp(ScriptEditor::get_singleton(), &ScriptEditor::open_script_create_dialog).bind("BTAction", String(GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_1")).path_join("new_task")));

	EditorFileSystem::get_singleton()->connect("resources_reload", callable_mp(this, &LimboAIEditor::_on_resources_reload));
}

LimboAIEditor::~LimboAIEditor() {
}

//**** LimboAIEditor ^

//**** LimboAIEditorPlugin

const Ref<Texture2D> LimboAIEditorPlugin::get_icon() const {
	return EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("LimboAIEditor"), SNAME("EditorIcons"));
}

void LimboAIEditorPlugin::apply_changes() {
	limbo_ai_editor->apply_changes();
}

void LimboAIEditorPlugin::_notification(int p_notification) {
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
	if (Object::cast_to<BehaviorTree>(p_object)) {
		return true;
	}
	return false;
}

LimboAIEditorPlugin::LimboAIEditorPlugin() {
	limbo_ai_editor = memnew(LimboAIEditor());
	limbo_ai_editor->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	EditorNode::get_singleton()->get_main_screen_control()->add_child(limbo_ai_editor);
	limbo_ai_editor->hide();
	add_debugger_plugin(memnew(LimboDebuggerPlugin));
}

LimboAIEditorPlugin::~LimboAIEditorPlugin() {
}

#endif // TOOLS_ENABLED
