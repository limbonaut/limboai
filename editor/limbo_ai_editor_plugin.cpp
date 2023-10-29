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

#include "action_banner.h"
#include "modules/limboai/bt/tasks/bt_comment.h"
#include "modules/limboai/bt/tasks/composites/bt_probability_selector.h"
#include "modules/limboai/bt/tasks/composites/bt_selector.h"
#include "modules/limboai/editor/debugger/limbo_debugger_plugin.h"
#include "modules/limboai/util/limbo_utility.h"

#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "editor/debugger/editor_debugger_node.h"
#include "editor/debugger/script_editor_debugger.h"
#include "editor/editor_file_system.h"
#include "editor/editor_help.h"
#include "editor/editor_paths.h"
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/inspector_dock.h"
#include "editor/plugins/script_editor_plugin.h"
#include "editor/project_settings_editor.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/separator.h"

//**** LimboAIEditor

_FORCE_INLINE_ String _get_script_template_path() {
	String templates_search_path = GLOBAL_GET("editor/script/templates_search_path");
	return templates_search_path.path_join("BTTask").path_join("custom_task.gd");
}

void LimboAIEditor::_add_task(const Ref<BTTask> &p_task) {
	if (task_tree->get_bt().is_null()) {
		return;
	}
	ERR_FAIL_COND(p_task.is_null());
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
	task_palette->show();

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

void LimboAIEditor::_create_user_task_dir() {
	String task_dir = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dir_1");
	ERR_FAIL_COND_MSG(DirAccess::exists(task_dir), "LimboAIEditor: Directory already exists: " + task_dir);

	Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	Error err;
	err = dir->make_dir_recursive(task_dir);
	ERR_FAIL_COND_MSG(err != OK, "LimboAIEditor: Failed to create directory: " + task_dir);

	EditorFileSystem::get_singleton()->scan_changes();
	_update_banners();
}

void LimboAIEditor::_edit_project_settings() {
	ProjectSettingsEditor::get_singleton()->set_general_page("limbo_ai/behavior_tree");
	ProjectSettingsEditor::get_singleton()->popup_project_settings();
	ProjectSettingsEditor::get_singleton()->connect(SNAME("visibility_changed"), callable_mp(this, &LimboAIEditor::_update_banners), CONNECT_ONE_SHOT);
}

void LimboAIEditor::_remove_task_from_favorite(const String &p_task) {
	PackedStringArray favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
	favorite_tasks.erase(p_task);
	ProjectSettings::get_singleton()->set_setting("limbo_ai/behavior_tree/favorite_tasks", favorite_tasks);
	ProjectSettings::get_singleton()->save();
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

	if (!(has_focus() || get_viewport()->gui_get_focus_owner() == nullptr || is_ancestor_of(get_viewport()->gui_get_focus_owner()))) {
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

	if (task_tree->selected_has_probability()) {
		menu->add_icon_item(theme_cache.percent_icon, TTR("Edit Probability"), ACTION_EDIT_PROBABILITY);
	}
	menu->add_icon_shortcut(theme_cache.rename_task_icon, ED_GET_SHORTCUT("limbo_ai/rename_task"), ACTION_RENAME);
	menu->add_icon_item(theme_cache.edit_script_icon, TTR("Edit Script"), ACTION_EDIT_SCRIPT);
	menu->add_icon_item(theme_cache.open_doc_icon, TTR("Open Documentation"), ACTION_OPEN_DOC);
	menu->set_item_disabled(menu->get_item_index(ACTION_EDIT_SCRIPT), task->get_script().is_null());

	menu->add_separator();
	menu->add_icon_shortcut(theme_cache.move_task_up_icon, ED_GET_SHORTCUT("limbo_ai/move_task_up"), ACTION_MOVE_UP);
	menu->add_icon_shortcut(theme_cache.move_task_down_icon, ED_GET_SHORTCUT("limbo_ai/move_task_down"), ACTION_MOVE_DOWN);
	menu->add_icon_shortcut(theme_cache.duplicate_task_icon, ED_GET_SHORTCUT("limbo_ai/duplicate_task"), ACTION_DUPLICATE);
	menu->add_icon_item(theme_cache.make_root_icon, TTR("Make Root"), ACTION_MAKE_ROOT);

	menu->add_separator();
	menu->add_icon_shortcut(theme_cache.remove_task_icon, ED_GET_SHORTCUT("limbo_ai/remove_task"), ACTION_REMOVE);

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
		case ACTION_EDIT_PROBABILITY: {
			Rect2 rect = task_tree->get_selected_probability_rect();
			ERR_FAIL_COND(rect == Rect2());
			rect.position.y += rect.size.y;
			rect.position += task_tree->get_rect().position;
			rect = task_tree->get_screen_transform().xform(rect);
			_update_probability_edit();
			probability_popup->popup(rect);
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

void LimboAIEditor::_on_probability_edited(double p_value) {
	Ref<BTTask> selected = task_tree->get_selected();
	ERR_FAIL_COND(selected == nullptr);
	Ref<BTProbabilitySelector> probability_selector = selected->get_parent();
	ERR_FAIL_COND(probability_selector.is_null());
	if (percent_mode->is_pressed()) {
		probability_selector->set_probability(probability_selector->get_child_index(selected), p_value * 0.01);
	} else {
		probability_selector->set_weight(probability_selector->get_child_index(selected), p_value);
	}
}

void LimboAIEditor::_update_probability_edit() {
	Ref<BTTask> selected = task_tree->get_selected();
	ERR_FAIL_COND(selected.is_null());
	Ref<BTProbabilitySelector> prob = selected->get_parent();
	ERR_FAIL_COND(prob.is_null());
	double others_weight = prob->get_total_weight() - prob->get_weight(prob->get_child_index(selected));
	bool cannot_edit_percent = others_weight == 0.0;
	percent_mode->set_disabled(cannot_edit_percent);
	if (cannot_edit_percent && percent_mode->is_pressed()) {
		weight_mode->set_pressed(true);
	}

	if (percent_mode->is_pressed()) {
		probability_edit->set_suffix("%");
		probability_edit->set_max(99.0);
		probability_edit->set_allow_greater(false);
		probability_edit->set_step(0.01);
		probability_edit->set_value_no_signal(task_tree->get_selected_probability_percent());
	} else {
		probability_edit->set_suffix("");
		probability_edit->set_allow_greater(true);
		probability_edit->set_max(10.0);
		probability_edit->set_step(0.01);
		probability_edit->set_value_no_signal(task_tree->get_selected_probability_weight());
	}
}

void LimboAIEditor::_probability_popup_closed() {
	probability_edit->grab_focus(); // Hack: Workaround for an EditorSpinSlider bug keeping LineEdit visible and "stuck" with ghost value.
}

void LimboAIEditor::_misc_option_selected(int p_id) {
	switch (p_id) {
		case MISC_INTRODUCTION: {
			ScriptEditor::get_singleton()->goto_help("class_name:BehaviorTree");
			EditorNode::get_singleton()->set_visible_editor(EditorNode::EDITOR_SCRIPT);
		} break;
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
			_edit_project_settings();
		} break;
		case MISC_CREATE_SCRIPT_TEMPLATE: {
			String template_path = _get_script_template_path();
			String template_dir = template_path.get_base_dir();

			if (!FileAccess::exists(template_path)) {
				Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_RESOURCES);
				Error err;
				if (!dir->exists(template_dir)) {
					err = dir->make_dir_recursive(template_dir);
					ERR_FAIL_COND(err != OK);
				}

				Ref<FileAccess> f = FileAccess::open(template_path, FileAccess::WRITE, &err);
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
						"func _tick(delta: float) -> Status:\n"
						"_TS_return SUCCESS\n";

				f->store_string(script_template);
				f->close();
			}

			ScriptEditor::get_singleton()->open_file(template_path);
		} break;
	}
}

void LimboAIEditor::_on_tree_task_selected(const Ref<BTTask> &p_task) {
	EditorNode::get_singleton()->edit_resource(p_task);
}

void LimboAIEditor::_on_visibility_changed() {
	if (task_tree->is_visible_in_tree()) {
		Ref<BTTask> sel = task_tree->get_selected();
		if (sel.is_valid()) {
			EditorNode::get_singleton()->edit_resource(sel);
		} else if (task_tree->get_bt().is_valid() && InspectorDock::get_inspector_singleton()->get_edited_object() != task_tree->get_bt().ptr()) {
			EditorNode::get_singleton()->edit_resource(task_tree->get_bt());
		}

		task_palette->refresh();
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
	ERR_FAIL_COND(p_type != 0 && p_to_task->get_parent().is_null());

	if (p_task == p_to_task) {
		return;
	}

	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Drag BT Task"));
	undo_redo->add_do_method(p_task->get_parent().ptr(), SNAME("remove_child"), p_task);

	if (p_type == 0) {
		undo_redo->add_do_method(p_to_task.ptr(), SNAME("add_child"), p_task);
		undo_redo->add_undo_method(p_to_task.ptr(), SNAME("remove_child"), p_task);
	} else if (p_type == -1) {
		undo_redo->add_do_method(p_to_task->get_parent().ptr(), SNAME("add_child_at_index"), p_task, p_to_task->get_parent()->get_child_index(p_to_task));
		undo_redo->add_undo_method(p_to_task->get_parent().ptr(), SNAME("remove_child"), p_task);
	} else if (p_type == 1) {
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

		if (task_meta.is_empty() || (!FileAccess::exists(task_meta) && !ClassDB::class_exists(task_meta))) {
			call_deferred(SNAME("_update_banners"));
			continue;
		}

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

	misc_menu->add_icon_item(theme_cache.open_doc_icon, TTR("Introduction"), MISC_INTRODUCTION);

	misc_menu->add_separator();
	misc_menu->add_icon_shortcut(theme_cache.open_debugger_icon, ED_GET_SHORTCUT("limbo_ai/open_debugger"), MISC_OPEN_DEBUGGER);
	misc_menu->add_item(TTR("Project Settings..."), MISC_PROJECT_SETTINGS);

	misc_menu->add_separator();
	misc_menu->add_item(
			FileAccess::exists(_get_script_template_path()) ? TTR("Edit Script Template") : TTR("Create Script Template"),
			MISC_CREATE_SCRIPT_TEMPLATE);
}

void LimboAIEditor::_update_banners() {
	for (int i = 0; i < banners->get_child_count(); i++) {
		if (banners->get_child(i)->has_meta(SNAME("managed"))) {
			banners->get_child(i)->queue_free();
		}
	}

	for (String dir_setting : { "limbo_ai/behavior_tree/user_task_dir_1", "limbo_ai/behavior_tree/user_task_dir_2", "limbo_ai/behavior_tree/user_task_dir_3" }) {
		String task_dir = GLOBAL_GET(dir_setting);
		if (!task_dir.is_empty() && !DirAccess::exists(task_dir)) {
			ActionBanner *banner = memnew(ActionBanner);
			banner->set_text(vformat(TTR("Task folder not found: %s"), task_dir));
			banner->add_action(TTR("Create"), callable_mp(this, &LimboAIEditor::_create_user_task_dir), true);
			banner->add_action(TTR("Edit Path..."), callable_mp(this, &LimboAIEditor::_edit_project_settings));
			banner->set_meta(SNAME("managed"), Variant(true));
			banners->call_deferred(SNAME("add_child"), banner);
		}
	}

	Array favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
	for (int i = 0; i < favorite_tasks.size(); i++) {
		String task_meta = favorite_tasks[i];

		if (task_meta.is_empty() || (!FileAccess::exists(task_meta) && !ClassDB::class_exists(task_meta))) {
			ActionBanner *banner = memnew(ActionBanner);
			banner->set_text(vformat(TTR("Favorite task not found: %s"), task_meta));
			banner->add_action(TTR("Remove"), callable_mp(this, &LimboAIEditor::_remove_task_from_favorite).bind(task_meta), true);
			banner->add_action(TTR("Edit Favorite Tasks..."), callable_mp(this, &LimboAIEditor::_edit_project_settings));
			banner->set_meta(SNAME("managed"), Variant(true));
			banners->call_deferred(SNAME("add_child"), banner);
		}
	}
}

void LimboAIEditor::_update_theme_item_cache() {
	Control::_update_theme_item_cache();

	theme_cache.duplicate_task_icon = get_theme_icon(SNAME("Duplicate"), SNAME("EditorIcons"));
	theme_cache.edit_script_icon = get_theme_icon(SNAME("Script"), SNAME("EditorIcons"));
	theme_cache.make_root_icon = get_theme_icon(SNAME("NewRoot"), SNAME("EditorIcons"));
	theme_cache.move_task_down_icon = get_theme_icon(SNAME("MoveDown"), SNAME("EditorIcons"));
	theme_cache.move_task_up_icon = get_theme_icon(SNAME("MoveUp"), SNAME("EditorIcons"));
	theme_cache.open_debugger_icon = get_theme_icon(SNAME("Debug"), SNAME("EditorIcons"));
	theme_cache.open_doc_icon = get_theme_icon(SNAME("Help"), SNAME("EditorIcons"));
	theme_cache.percent_icon = get_theme_icon(SNAME("LimboPercent"), SNAME("EditorIcons"));
	theme_cache.remove_task_icon = get_theme_icon(SNAME("Remove"), SNAME("EditorIcons"));
	theme_cache.rename_task_icon = get_theme_icon(SNAME("Rename"), SNAME("EditorIcons"));
}

void LimboAIEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			Ref<ConfigFile> cf;
			cf.instantiate();
			String conf_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("limbo_ai.cfg");
			if (cf->load(conf_path) == OK) {
				hsc->set_split_offset(cf->get_value("bt_editor", "bteditor_hsplit", hsc->get_split_offset()));
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			Ref<ConfigFile> cf;
			cf.instantiate();
			String conf_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("limbo_ai.cfg");
			cf->load(conf_path);
			cf->set_value("bt_editor", "bteditor_hsplit", hsc->get_split_offset());
			cf->save(conf_path);
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			new_btn->set_icon(get_theme_icon(SNAME("New"), SNAME("EditorIcons")));
			load_btn->set_icon(get_theme_icon(SNAME("Load"), SNAME("EditorIcons")));
			save_btn->set_icon(get_theme_icon(SNAME("Save"), SNAME("EditorIcons")));
			new_script_btn->set_icon(get_theme_icon(SNAME("ScriptCreate"), SNAME("EditorIcons")));
			history_back->set_icon(get_theme_icon(SNAME("Back"), SNAME("EditorIcons")));
			history_forward->set_icon(get_theme_icon(SNAME("Forward"), SNAME("EditorIcons")));
			misc_btn->set_icon(get_theme_icon(SNAME("Tools"), SNAME("EditorIcons")));

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

	vbox = memnew(VBoxContainer);
	vbox->set_anchor(SIDE_RIGHT, ANCHOR_END);
	vbox->set_anchor(SIDE_BOTTOM, ANCHOR_END);
	add_child(vbox);

	HBoxContainer *toolbar = memnew(HBoxContainer);
	vbox->add_child(toolbar);

	PackedStringArray favorite_tasks_default;
	favorite_tasks_default.append("BTSelector");
	favorite_tasks_default.append("BTSequence");
	favorite_tasks_default.append("BTComment");
	GLOBAL_DEF(PropertyInfo(Variant::PACKED_STRING_ARRAY, "limbo_ai/behavior_tree/favorite_tasks", PROPERTY_HINT_ARRAY_TYPE, "String"), favorite_tasks_default);

	fav_tasks_hbox = memnew(HBoxContainer);
	toolbar->add_child(fav_tasks_hbox);

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

	misc_btn = memnew(MenuButton);
	misc_btn->set_text(TTR("Misc"));
	misc_btn->set_flat(true);
	misc_btn->connect("pressed", callable_mp(this, &LimboAIEditor::_update_misc_menu));
	misc_btn->get_popup()->connect("id_pressed", callable_mp(this, &LimboAIEditor::_misc_option_selected));
	toolbar->add_child(misc_btn);

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
	vbox->add_child(header);

	hsc = memnew(HSplitContainer);
	hsc->set_h_size_flags(SIZE_EXPAND_FILL);
	hsc->set_v_size_flags(SIZE_EXPAND_FILL);
	hsc->set_focus_mode(FOCUS_NONE);
	vbox->add_child(hsc);

	task_tree = memnew(TaskTree);
	task_tree->set_v_size_flags(SIZE_EXPAND_FILL);
	task_tree->set_h_size_flags(SIZE_EXPAND_FILL);
	task_tree->hide();
	task_tree->connect("rmb_pressed", callable_mp(this, &LimboAIEditor::_on_tree_rmb));
	task_tree->connect("task_selected", callable_mp(this, &LimboAIEditor::_on_tree_task_selected));
	task_tree->connect("task_dragged", callable_mp(this, &LimboAIEditor::_on_task_dragged));
	task_tree->connect("task_activated", callable_mp(this, &LimboAIEditor::_action_selected).bind(ACTION_RENAME));
	task_tree->connect("probability_clicked", callable_mp(this, &LimboAIEditor::_action_selected).bind(ACTION_EDIT_PROBABILITY));
	task_tree->connect("visibility_changed", callable_mp(this, &LimboAIEditor::_on_visibility_changed));
	task_tree->connect("visibility_changed", callable_mp(this, &LimboAIEditor::_update_banners));
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

	task_palette = memnew(TaskPalette());
	hsc->set_split_offset(-300);
	task_palette->connect("task_selected", callable_mp(this, &LimboAIEditor::_add_task_by_class_or_path));
	task_palette->connect("favorite_tasks_changed", callable_mp(this, &LimboAIEditor::_update_favorite_tasks));
	task_palette->hide();
	hsc->add_child(task_palette);

	banners = memnew(VBoxContainer);
	vbox->add_child(banners);

	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", callable_mp(this, &LimboAIEditor::_action_selected));

	probability_popup = memnew(PopupPanel);
	{
		VBoxContainer *vbc = memnew(VBoxContainer);
		probability_popup->add_child(vbc);

		PanelContainer *mode_panel = memnew(PanelContainer);
		vbc->add_child(mode_panel);

		HBoxContainer *mode_hbox = memnew(HBoxContainer);
		mode_panel->add_child(mode_hbox);

		Ref<ButtonGroup> button_group;
		button_group.instantiate();

		weight_mode = memnew(Button);
		mode_hbox->add_child(weight_mode);
		weight_mode->set_toggle_mode(true);
		weight_mode->set_button_group(button_group);
		weight_mode->set_focus_mode(Control::FOCUS_NONE);
		weight_mode->set_text(TTR("Weight"));
		weight_mode->set_tooltip_text(TTR("Edit weight"));
		weight_mode->connect("pressed", callable_mp(this, &LimboAIEditor::_update_probability_edit));
		weight_mode->set_pressed_no_signal(true);

		percent_mode = memnew(Button);
		mode_hbox->add_child(percent_mode);
		percent_mode->set_toggle_mode(true);
		percent_mode->set_button_group(button_group);
		percent_mode->set_focus_mode(Control::FOCUS_NONE);
		percent_mode->set_text(TTR("Percent"));
		percent_mode->set_tooltip_text(TTR("Edit percent"));
		percent_mode->connect("pressed", callable_mp(this, &LimboAIEditor::_update_probability_edit));

		probability_edit = memnew(EditorSpinSlider);
		vbc->add_child(probability_edit);
		probability_edit->set_min(0.0);
		probability_edit->set_max(10.0);
		probability_edit->set_step(0.01);
		probability_edit->set_allow_greater(true);
		probability_edit->set_custom_minimum_size(Size2(200.0 * EDSCALE, 0.0));
		probability_edit->connect("value_changed", callable_mp(this, &LimboAIEditor::_on_probability_edited));

		probability_popup->connect("popup_hide", callable_mp(this, &LimboAIEditor::_probability_popup_closed));
	}
	add_child(probability_popup);

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
