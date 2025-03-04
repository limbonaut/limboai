/**
 * limbo_ai_editor_plugin.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#include "limbo_ai_editor_plugin.h"

#include "../bt/behavior_tree.h"
#include "../bt/tasks/bt_comment.h"
#include "../bt/tasks/composites/bt_probability_selector.h"
#include "../bt/tasks/composites/bt_selector.h"
#include "../bt/tasks/decorators/bt_subtree.h"
#include "../compat/editor.h"
#include "../compat/editor_scale.h"
#include "../compat/editor_settings.h"
#include "../compat/limbo_compat.h"
#include "../compat/object.h"
#include "../compat/project_settings.h"
#include "../compat/resource.h"
#include "../compat/resource_loader.h"
#include "../compat/scene_tree.h"
#include "../compat/translation.h"
#include "../compat/variant.h"
#include "../util/limbo_utility.h"
#include "../util/limboai_version.h"
#include "action_banner.h"
#include "blackboard_plan_editor.h"
#include "debugger/limbo_debugger_plugin.h"
#include "editor_property_bb_param.h"
#include "editor_property_property_path.h"
#include "editor_property_variable_name.h"

#ifdef LIMBOAI_MODULE
#include "core/input/input.h"
#include "editor/debugger/editor_debugger_node.h"
#include "editor/debugger/script_editor_debugger.h"
#include "editor/editor_node.h"
#include "editor/filesystem_dock.h"
#include "editor/gui/editor_bottom_panel.h"
#include "editor/plugins/script_editor_plugin.h"
#include "editor/project_settings_editor.h"
#include "scene/gui/separator.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/button_group.hpp>
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/file_system_dock.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/script_editor.hpp>
#include <godot_cpp/classes/v_separator.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

namespace {

// If built-in resource - switch to the owner scene (open it if not already).
inline void _switch_to_owner_scene_if_builtin(const Ref<BehaviorTree> &p_behavior_tree) {
	if (p_behavior_tree.is_valid() && p_behavior_tree->get_path().contains("::")) {
		String current_scene = SCENE_TREE()->get_edited_scene_root()->get_scene_file_path();
		String scene_path = p_behavior_tree->get_path().get_slice("::", 0);
		if (current_scene != scene_path) {
			EditorInterface::get_singleton()->open_scene_from_path(scene_path);
		}
	}
}

} // unnamed namespace

//**** LimboAIEditor

_FORCE_INLINE_ String _get_script_template_path() {
	String templates_search_path = GLOBAL_GET("editor/script/templates_search_path");
	return templates_search_path.path_join("BTTask").path_join("custom_task.gd");
}

EditorUndoRedoManager *LimboAIEditor::_new_undo_redo_action(const String &p_name, UndoRedo::MergeMode p_mode) {
#ifdef LIMBOAI_MODULE
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
#elif LIMBOAI_GDEXTENSION
	EditorUndoRedoManager *undo_redo = plugin->get_undo_redo();
#endif
	// ! HACK: Force global history to be used for resources without a set path.
	undo_redo->create_action(p_name, p_mode, dummy_history_context);
	undo_redo->force_fixed_history();
	return undo_redo;
}

void LimboAIEditor::_commit_action_with_update(EditorUndoRedoManager *p_undo_redo) {
	ERR_FAIL_NULL(p_undo_redo);
	p_undo_redo->add_do_method(this, LW_NAME(_update_task_tree), task_tree->get_bt());
	p_undo_redo->add_undo_method(this, LW_NAME(_update_task_tree), task_tree->get_bt());
	p_undo_redo->commit_action();
	_set_as_dirty(task_tree->get_bt(), true);
}

void LimboAIEditor::_add_task(const Ref<BTTask> &p_task, bool p_as_sibling) {
	if (task_tree->get_bt().is_null()) {
		return;
	}
	ERR_FAIL_COND(p_task.is_null());

	EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Add BT Task"));

	int insert_idx = -1;
	Ref<BTTask> selected = task_tree->get_selected();
	Ref<BTTask> parent = selected;
	if (parent.is_null()) {
		// When no task is selected, use the root task.
		parent = task_tree->get_bt()->get_root_task();
		selected = parent;
	}
	if (parent.is_null()) {
		// When tree is empty.
		undo_redo->add_do_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), p_task);
		undo_redo->add_undo_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), task_tree->get_bt()->get_root_task());
	} else {
		if (p_as_sibling && selected.is_valid() && selected->get_parent().is_valid()) {
			// Insert task after the currently selected and on the same level (usually when shift is pressed).
			parent = selected->get_parent();
			insert_idx = selected->get_index() + 1;
		}
		undo_redo->add_do_method(parent.ptr(), LW_NAME(add_child_at_index), p_task, insert_idx);
		undo_redo->add_undo_method(parent.ptr(), LW_NAME(remove_child), p_task);
	}
	_commit_action_with_update(undo_redo);
}

void LimboAIEditor::_add_task_with_prototype(const Ref<BTTask> &p_prototype) {
	Ref<BTTask> selected = task_tree->get_selected();
	bool as_sibling = Input::get_singleton()->is_key_pressed(LW_KEY(SHIFT));
	_add_task(p_prototype->clone(), as_sibling);
}

Ref<BTTask> LimboAIEditor::_create_task_by_class_or_path(const String &p_class_or_path) const {
	ERR_FAIL_COND_V(p_class_or_path.is_empty(), nullptr);

	Ref<BTTask> ret;

	if (p_class_or_path.begins_with("res:")) {
		Ref<Script> s = RESOURCE_LOAD(p_class_or_path, "Script");
		ERR_FAIL_COND_V_MSG(s.is_null(), nullptr, vformat("LimboAI: Can't add task. Bad script: %s", p_class_or_path));
		StringName base_type = s->get_instance_base_type();
		if (base_type == StringName()) {
			// Try reloading script.
			s->reload(true);
			base_type = s->get_instance_base_type();
		}
		ERR_FAIL_COND_V_MSG(base_type == StringName(), nullptr, vformat("LimboAI: Can't add task. Bad script: %s", p_class_or_path));

		Variant inst = ClassDB::instantiate(base_type);
		Object *obj = inst;
		ERR_FAIL_NULL_V_MSG(obj, nullptr, vformat("LimboAI: Can't add task. Failed to create base type \"%s\".", base_type));

		if (unlikely(!IS_CLASS(obj, BTTask))) {
			ERR_PRINT_ED(vformat("LimboAI: Can't add task. Script is not a BTTask: %s", p_class_or_path));
			VARIANT_DELETE_IF_OBJECT(inst);
			return nullptr;
		}

		ret.reference_ptr(Object::cast_to<BTTask>(obj));
		ret->set_script(s);
	} else {
		ERR_FAIL_COND_V(!ClassDB::is_parent_class(p_class_or_path, "BTTask"), nullptr);
		ret = ClassDB::instantiate(p_class_or_path);
	}
	return ret;
}

void LimboAIEditor::_add_task_by_class_or_path(const String &p_class_or_path) {
	Ref<BTTask> selected = task_tree->get_selected();
	bool as_sibling = Input::get_singleton()->is_key_pressed(LW_KEY(SHIFT));
	_add_task(_create_task_by_class_or_path(p_class_or_path), as_sibling);
}

void LimboAIEditor::_remove_task(const Ref<BTTask> &p_task) {
	ERR_FAIL_COND(p_task.is_null());
	ERR_FAIL_COND(task_tree->get_bt().is_null());
	EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Remove BT Task"));
	if (p_task->get_parent().is_null()) {
		ERR_FAIL_COND(task_tree->get_bt()->get_root_task() != p_task);
		undo_redo->add_do_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), Variant());
		undo_redo->add_undo_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), task_tree->get_bt()->get_root_task());
	} else {
		undo_redo->add_do_method(p_task->get_parent().ptr(), LW_NAME(remove_child), p_task);
		undo_redo->add_undo_method(p_task->get_parent().ptr(), LW_NAME(add_child_at_index), p_task, p_task->get_index());
	}
	_commit_action_with_update(undo_redo);
}

void LimboAIEditor::_new_bt() {
	Ref<BehaviorTree> bt = memnew(BehaviorTree);
	bt->set_root_task(memnew(BTSelector));
	bt->set_blackboard_plan(memnew(BlackboardPlan));
	EditorInterface::get_singleton()->edit_resource(bt);
}

void LimboAIEditor::_save_bt(const Ref<BehaviorTree> &p_bt, const String &p_path) {
	ERR_FAIL_COND(p_path.is_empty());
	ERR_FAIL_COND(!p_path.begins_with("res://"));
	ERR_FAIL_COND(p_bt.is_null());

	if (p_bt->get_path() != p_path) {
#ifdef LIMBOAI_MODULE
		task_tree->get_bt()->set_path(p_path, true);
#elif LIMBOAI_GDEXTENSION
		task_tree->get_bt()->take_over_path(p_path);
#endif
	}

	// This is a workaround, because EditorNode::save_resource() function is not accessible in GDExtension.
	if (RESOURCE_IS_BUILT_IN(p_bt)) {
		// If built-in resource - save the containing resource instead.
		String file_path = p_path.get_slice("::", 0);
		ERR_FAIL_COND_MSG(!RESOURCE_EXISTS(file_path, "Resource"), "LimboAI: SAVE FAILED - resource file doesn't exist: " + file_path);
		if (RESOURCE_IS_SCENE_FILE(file_path)) {
			// Packed scene - save the scene instead.
			if (EditorInterface::get_singleton()->get_open_scenes().has(file_path)) {
				// If scene is open, switch to it first, and then ask to save.
				// This is needed because saving the currently edited scene can have complications.
				EditorInterface::get_singleton()->open_scene_from_path(file_path);
				EditorInterface::get_singleton()->save_scene();
			} else {
				// If scene is not currently open in the editor, load and resave it.
				Ref<Resource> scene = RESOURCE_LOAD(file_path, "PackedScene");
				RESOURCE_SAVE(scene, file_path, ResourceSaver::FLAG_NONE);
			}
		} else {
			// Not a packed scene - save the containing resource to file.
			Ref<Resource> res = RESOURCE_LOAD(file_path, "Resource");
			RESOURCE_SAVE(res, file_path, ResourceSaver::FLAG_NONE);
		}
	} else {
		// If external resource - save to file.
		RESOURCE_SAVE(p_bt, p_path, ResourceSaver::FLAG_CHANGE_PATH);
	}

	_set_as_dirty(p_bt, false);
	_update_tabs();
}

void LimboAIEditor::_save_current_bt(const String &p_path) {
	ERR_FAIL_COND_MSG(p_path.is_empty(), "LimboAI: SAVE FAILED - p_path is empty");
	ERR_FAIL_COND_MSG(task_tree->get_bt().is_null(), "LimboAI: SAVE FAILED - bt is null");

	_save_bt(task_tree->get_bt(), p_path);
}

void LimboAIEditor::_load_bt(const String &p_path) {
	ERR_FAIL_COND_MSG(p_path.is_empty(), "Empty p_path");
	Ref<BehaviorTree> bt = RESOURCE_LOAD(p_path, "BehaviorTree");
	ERR_FAIL_COND(!bt.is_valid());
	if (bt->get_blackboard_plan().is_null()) {
		bt->set_blackboard_plan(memnew(BlackboardPlan));
	}
	// if (history.find(bt) != -1) {
	// 	history.erase(bt);
	// 	history.push_back(bt);
	// }

	EditorInterface::get_singleton()->edit_resource(bt);
}

void LimboAIEditor::_update_task_tree(const Ref<BehaviorTree> &p_bt, const Ref<BTTask> &p_specific_task) {
	ERR_FAIL_COND(p_bt.is_null());
	if (task_tree->get_bt() == p_bt) {
		if (p_specific_task.is_null()) {
			task_tree->update_tree();
		} else {
			task_tree->update_task(p_specific_task);
		}
	} else {
		// The given BT is not being displayed - open it.
		edit_bt(p_bt);
	}
}

void LimboAIEditor::_disable_editing() {
	task_tree->unload();
	task_palette->hide();
	task_tree->hide();
	usage_hint->show();
}

void LimboAIEditor::edit_bt(const Ref<BehaviorTree> &p_behavior_tree, bool p_force_refresh) {
	ERR_FAIL_COND_MSG(p_behavior_tree.is_null(), "p_behavior_tree is null");

	_switch_to_owner_scene_if_builtin(p_behavior_tree);

	if (!p_force_refresh && task_tree->get_bt() == p_behavior_tree) {
		return;
	}

#ifdef LIMBOAI_MODULE
	p_behavior_tree->editor_set_section_unfold("blackboard_plan", true);
	p_behavior_tree->notify_property_list_changed();
#endif // LIMBOAI_MODULE
	// Remember current search info.
	if (idx_history >= 0 && idx_history < history.size() && task_tree->get_bt() == history[idx_history]) {
		tab_search_context.insert(history[idx_history], task_tree->tree_search_get_search_info());
	}

	task_tree->load_bt(p_behavior_tree);

	if (task_tree->get_bt().is_valid() && !task_tree->get_bt()->is_connected(LW_NAME(changed), callable_mp(this, &LimboAIEditor::_set_as_dirty))) {
		task_tree->get_bt()->connect(LW_NAME(changed), callable_mp(this, &LimboAIEditor::_set_as_dirty).bind(task_tree->get_bt(), true));
	}

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

	// Restore search info from [tab_search_context].
	if (idx_history >= 0 && idx_history < history.size()) {
		if (tab_search_context.has(history[idx_history])) {
			task_tree->tree_search_set_search_info(tab_search_context[history[idx_history]]);
		} else {
			task_tree->tree_search_set_search_info(TreeSearch::SearchInfo());
		}
	}

	_update_tabs();
}

Ref<BlackboardPlan> LimboAIEditor::get_edited_blackboard_plan() {
	if (task_tree->get_bt().is_null()) {
		return nullptr;
	}
	if (task_tree->get_bt()->get_blackboard_plan().is_null()) {
		task_tree->get_bt()->set_blackboard_plan(memnew(BlackboardPlan));
	}
	return task_tree->get_bt()->get_blackboard_plan();
}

void LimboAIEditor::set_window_layout(const Ref<ConfigFile> &p_configuration) {
	Array open_bts;
	open_bts = p_configuration->get_value("LimboAI", "bteditor_open_bts", open_bts);
	for (int i = 0; i < open_bts.size(); i++) {
		String path = open_bts[i];
		if (FILE_EXISTS(path)) {
			_load_bt(path);
		}
	}

	hsc->set_split_offset(p_configuration->get_value("LimboAI", "bteditor_hsplit", hsc->get_split_offset()));
}

void LimboAIEditor::get_window_layout(const Ref<ConfigFile> &p_configuration) {
	Array open_bts;
	for (const Ref<BehaviorTree> &bt : history) {
		open_bts.push_back(bt->get_path());
	}
	p_configuration->set_value("LimboAI", "bteditor_open_bts", open_bts);

	int split_offset = hsc->get_split_offset();
	if (editor_layout != (int)EDITOR_GET("limbo_ai/editor/layout")) {
		// Editor layout settings changed - flip split offset.
		split_offset *= -1;
	}
	p_configuration->set_value("LimboAI", "bteditor_hsplit", split_offset);
}

void LimboAIEditor::_set_as_dirty(const Ref<BehaviorTree> &p_bt, bool p_dirty) {
	if (p_dirty && !dirty.has(p_bt)) {
		dirty.insert(p_bt);
	} else if (p_dirty == false && dirty.has(p_bt)) {
		dirty.erase(p_bt);
	}
}

void LimboAIEditor::_create_user_task_dir(String task_dir) {
	ERR_FAIL_COND_MSG(DirAccess::dir_exists_absolute(task_dir), "LimboAIEditor: Directory already exists: " + task_dir);

	Error err = DirAccess::make_dir_recursive_absolute(task_dir);
	ERR_FAIL_COND_MSG(err != OK, "LimboAIEditor: Failed to create directory: " + task_dir);

#ifdef LIMBOAI_MODULE
	EditorFileSystem::get_singleton()->scan_changes();
#elif LIMBOAI_GDEXTENSION
	EditorInterface::get_singleton()->get_resource_filesystem()->scan_sources();
#endif
	_update_banners();
}

void LimboAIEditor::_edit_project_settings() {
#ifdef LIMBOAI_MODULE
	ProjectSettingsEditor::get_singleton()->set_general_page("limbo_ai/behavior_tree");
	ProjectSettingsEditor::get_singleton()->popup_project_settings();
	ProjectSettingsEditor::get_singleton()->connect(LW_NAME(visibility_changed), callable_mp(this, &LimboAIEditor::_update_banners), CONNECT_ONE_SHOT);
#elif LIMBOAI_GDEXTENSION
	// TODO: Find a way to show project setting in GDExtension.
	String text = "Can't open settings in GDExtension, sorry :(\n"
				  "To edit project settings, navigate to \"Project->Project Settings\",\n"
				  "enable \"Advanced settings\", and scroll down to the \"LimboAI\" section.";
	_popup_info_dialog(text);
#endif
}

void LimboAIEditor::_remove_task_from_favorite(const String &p_task) {
	PackedStringArray favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
	int idx = favorite_tasks.find(p_task);
	if (idx >= 0) {
		favorite_tasks.remove_at(idx);
	}
	ProjectSettings::get_singleton()->set_setting("limbo_ai/behavior_tree/favorite_tasks", favorite_tasks);
	ProjectSettings::get_singleton()->save();
}

void LimboAIEditor::_save_and_restart() {
	ProjectSettings::get_singleton()->save();
	EditorInterface::get_singleton()->save_all_scenes();
	EditorInterface::get_singleton()->restart_editor(true);
}

void LimboAIEditor::_extract_subtree(const String &p_path) {
	Ref<BTTask> selected = task_tree->get_selected();
	ERR_FAIL_COND(selected.is_null());

	EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Extract Subtree"));

	Ref<BehaviorTree> bt = memnew(BehaviorTree);
	bt->set_root_task(selected->clone());
	bt->set_path(p_path);
	RESOURCE_SAVE(bt, p_path, ResourceSaver::FLAG_CHANGE_PATH);

	Ref<BTSubtree> subtree = memnew(BTSubtree);
	subtree->set_subtree(bt);

	if (selected->is_root()) {
		undo_redo->add_do_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), subtree);
		undo_redo->add_undo_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), selected);
	} else {
		int idx = selected->get_index();
		undo_redo->add_do_method(selected->get_parent().ptr(), LW_NAME(remove_child), selected);
		undo_redo->add_do_method(selected->get_parent().ptr(), LW_NAME(add_child_at_index), subtree, idx);
		undo_redo->add_undo_method(selected->get_parent().ptr(), LW_NAME(remove_child), subtree);
		undo_redo->add_undo_method(selected->get_parent().ptr(), LW_NAME(add_child_at_index), selected, idx);
	}
	_commit_action_with_update(undo_redo);
	EditorInterface::get_singleton()->edit_resource(task_tree->get_selected());
}

void LimboAIEditor::_process_shortcut_input(const Ref<InputEvent> &p_event) {
	if (!p_event->is_pressed() || p_event->is_echo()) {
		return;
	}

	bool handled = false;

	// * Global shortcuts.

	if (LW_IS_SHORTCUT("limbo_ai/open_debugger", p_event)) {
		_misc_option_selected(MISC_OPEN_DEBUGGER);
		handled = true;
	}

	// * When editor is on screen.

	if (!handled && is_visible_in_tree()) {
		if (LW_IS_SHORTCUT("limbo_ai/jump_to_owner", p_event)) {
			_tab_menu_option_selected(TAB_JUMP_TO_OWNER);
			handled = true;
		} else if (LW_IS_SHORTCUT("limbo_ai/close_tab", p_event)) {
			_tab_menu_option_selected(TAB_CLOSE);
			handled = true;
		} else if (LW_IS_SHORTCUT("limbo_ai/editor_save_scene", p_event)) {
			// This intercepts the editor save action, but does not set the event as handled because we don't know the user's intention.
			// We just want to save the currently edited BT as well, which may cause a loop with built-in resource if done from "_save_external_data".
			// Workaround for: https://github.com/limbonaut/limboai/issues/240#issuecomment-2453087424
			if (task_tree->get_bt().is_valid() && RESOURCE_IS_BUILT_IN(task_tree->get_bt())) {
				_on_save_pressed();
			}
			handled = false; // intentionally not set as handled
		}
	}

	// * When editor is focused.

	if (!handled && (has_focus() || (get_viewport()->gui_get_focus_owner() && is_ancestor_of(get_viewport()->gui_get_focus_owner())))) {
		handled = true;
		if (LW_IS_SHORTCUT("limbo_ai/rename_task", p_event)) {
			_action_selected(ACTION_RENAME);
		} else if (LW_IS_SHORTCUT("limbo_ai/cut_task", p_event)) {
			_action_selected(ACTION_CUT);
		} else if (LW_IS_SHORTCUT("limbo_ai/copy_task", p_event)) {
			_action_selected(ACTION_COPY);
		} else if (LW_IS_SHORTCUT("limbo_ai/paste_task", p_event)) {
			_action_selected(ACTION_PASTE);
		} else if (LW_IS_SHORTCUT("limbo_ai/paste_task_after", p_event)) {
			_action_selected(ACTION_PASTE_AFTER);
		} else if (LW_IS_SHORTCUT("limbo_ai/move_task_up", p_event)) {
			_action_selected(ACTION_MOVE_UP);
		} else if (LW_IS_SHORTCUT("limbo_ai/move_task_down", p_event)) {
			_action_selected(ACTION_MOVE_DOWN);
		} else if (LW_IS_SHORTCUT("limbo_ai/duplicate_task", p_event)) {
			_action_selected(ACTION_DUPLICATE);
		} else if (LW_IS_SHORTCUT("limbo_ai/remove_task", p_event)) {
			_action_selected(ACTION_REMOVE);
		} else if (LW_IS_SHORTCUT("limbo_ai/new_behavior_tree", p_event)) {
			_new_bt();
		} else if (LW_IS_SHORTCUT("limbo_ai/save_behavior_tree", p_event)) {
			_on_save_pressed();
		} else if (LW_IS_SHORTCUT("limbo_ai/load_behavior_tree", p_event)) {
			_popup_file_dialog(load_dialog);
		} else if (LW_IS_SHORTCUT("limbo_ai/find_task", p_event)) {
			task_tree->tree_search_show_and_focus();
		} else {
			handled = false;
		}
	}

	if (handled) {
		get_viewport()->set_input_as_handled();
	}
}

void LimboAIEditor::_on_tree_rmb(const Vector2 &p_menu_pos) {
	menu->clear();

	Vector<Ref<BTTask>> selection = task_tree->get_selected_tasks();
	if (selection.is_empty()) {
		return;
	}

	if (selection.size() == 1 && task_tree->selected_has_probability()) {
		menu->add_icon_item(theme_cache.percent_icon, TTR("Edit Probability"), ACTION_EDIT_PROBABILITY);
	}
	menu->add_icon_shortcut(theme_cache.rename_task_icon, LW_GET_SHORTCUT("limbo_ai/rename_task"), ACTION_RENAME);
	menu->add_icon_item(theme_cache.change_type_icon, TTR("Change Type"), ACTION_CHANGE_TYPE);
	menu->add_icon_item(theme_cache.edit_script_icon, TTR("Edit Script"), ACTION_EDIT_SCRIPT);
	menu->add_icon_item(theme_cache.doc_icon, TTR("Open Documentation"), ACTION_OPEN_DOC);
	menu->set_item_disabled(menu->get_item_index(ACTION_RENAME), selection.size() != 1);
	menu->set_item_disabled(menu->get_item_index(ACTION_CHANGE_TYPE), selection.size() != 1);
	menu->set_item_disabled(menu->get_item_index(ACTION_EDIT_SCRIPT), selection.size() != 1 || selection[0]->get_script() == Variant());
	menu->set_item_disabled(menu->get_item_index(ACTION_OPEN_DOC), selection.size() != 1);

	menu->add_separator();
	{
		int acc = 0;
		for (const Ref<BTTask> &task : selection) {
			acc += task->is_enabled() ? 1 : -1;
		}
		Ref<Texture2D> icon;
		if (Math::abs(acc) == selection.size() && acc > 0) {
			icon = theme_cache.checked_icon;
		} else if (Math::abs(acc) == selection.size() && acc < 0) {
			icon = theme_cache.unchecked_icon;
		} else {
			icon = theme_cache.indeterminate_icon;
		}
		menu->add_icon_item(icon, TTR("Enabled"), ACTION_ENABLED);
		menu->set_item_checked(menu->get_item_index(ACTION_ENABLED), selection[0]->is_enabled());
	}

	menu->add_separator();
	menu->add_icon_shortcut(theme_cache.cut_icon, LW_GET_SHORTCUT("limbo_ai/cut_task"), ACTION_CUT);
	menu->add_icon_shortcut(theme_cache.copy_icon, LW_GET_SHORTCUT("limbo_ai/copy_task"), ACTION_COPY);
	menu->add_icon_shortcut(theme_cache.paste_icon, LW_GET_SHORTCUT("limbo_ai/paste_task"), ACTION_PASTE);
	menu->add_icon_shortcut(theme_cache.paste_icon, LW_GET_SHORTCUT("limbo_ai/paste_task_after"), ACTION_PASTE_AFTER);
	menu->set_item_disabled(menu->get_item_index(ACTION_PASTE), clipboard.is_empty() || selection.size() != 1);
	menu->set_item_disabled(menu->get_item_index(ACTION_PASTE_AFTER), clipboard.is_empty() || selection.size() != 1 || selection[0] == task_tree->get_bt()->get_root_task());

	menu->add_separator();
	menu->add_icon_shortcut(theme_cache.move_task_up_icon, LW_GET_SHORTCUT("limbo_ai/move_task_up"), ACTION_MOVE_UP);
	menu->add_icon_shortcut(theme_cache.move_task_down_icon, LW_GET_SHORTCUT("limbo_ai/move_task_down"), ACTION_MOVE_DOWN);
	menu->add_icon_shortcut(theme_cache.duplicate_task_icon, LW_GET_SHORTCUT("limbo_ai/duplicate_task"), ACTION_DUPLICATE);
	menu->add_icon_item(theme_cache.make_root_icon, TTR("Make Root"), ACTION_MAKE_ROOT);
	menu->add_icon_item(theme_cache.extract_subtree_icon, TTR("Extract Subtree"), ACTION_EXTRACT_SUBTREE);
	menu->set_item_disabled(menu->get_item_index(ACTION_MAKE_ROOT), selection.size() != 1);
	menu->set_item_disabled(menu->get_item_index(ACTION_EXTRACT_SUBTREE), selection.size() != 1);

	menu->add_separator();
	menu->add_icon_shortcut(theme_cache.remove_task_icon, LW_GET_SHORTCUT("limbo_ai/remove_task"), ACTION_REMOVE);

	menu->reset_size();
	menu->set_position(p_menu_pos);
	menu->popup();
}

void LimboAIEditor::_action_selected(int p_id) {
	switch (p_id) {
		case ACTION_RENAME: {
			Ref<BTTask> task = task_tree->get_selected();
			if (task.is_null()) {
				return;
			}
			if (IS_CLASS(task, BTComment)) {
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
		case ACTION_ENABLED: {
			Vector<Ref<BTTask>> selection = task_tree->get_selected_tasks();
			if (selection.is_empty()) {
				return;
			}
			bool tasks_enabled = false;
			for (const Ref<BTTask> &task : selection) {
				if (task->is_enabled()) {
					tasks_enabled = true;
					break;
				}
			}
			for (int i = selection.size() - 1; i >= 0; i--) {
				if (selection[i]->is_enabled() != tasks_enabled) {
					selection.remove_at(i);
				}
			}
			EditorUndoRedoManager *undo_redo = _new_undo_redo_action(
					tasks_enabled ? TTR("Disable BT tasks") : TTR("Enable BT tasks"));
			for (const Ref<BTTask> &task : selection) {
				ERR_CONTINUE(task.is_null());
				undo_redo->add_do_method(task.ptr(), LW_NAME(_set_enabled), !tasks_enabled);
				undo_redo->add_undo_method(task.ptr(), LW_NAME(_set_enabled), tasks_enabled);
			}
			undo_redo->commit_action();
			_set_as_dirty(task_tree->get_bt(), true);
		} break;
		case ACTION_CHANGE_TYPE: {
			change_type_palette->clear_filter();
			change_type_palette->refresh();
			Rect2 rect = Rect2(get_global_mouse_position(), Size2(400.0, 600.0) * EDSCALE);
			change_type_popup->popup(rect);
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
			EditorInterface::get_singleton()->edit_resource(task_tree->get_selected()->get_script());
		} break;
		case ACTION_OPEN_DOC: {
			Ref<BTTask> task = task_tree->get_selected();
			ERR_FAIL_COND(task.is_null());
			String help_class;

			Ref<Script> sc = GET_SCRIPT(task);
			if (sc.is_valid() && sc->get_path().is_absolute_path()) {
				help_class = sc->get_path();
			}
			if (help_class.is_empty()) {
				// Assuming context task is core class.
				help_class = task->get_class();
			}

			LimboUtility::get_singleton()->open_doc_class(help_class);
		} break;
		case ACTION_COPY: {
			Vector<Ref<BTTask>> selection = task_tree->get_selected_tasks();
			if (selection.is_empty()) {
				return;
			}
			clipboard.clear();
			for (const Ref<BTTask> &task : selection) {
				clipboard.push_back(task->clone());
			}
		} break;
		case ACTION_PASTE:
		case ACTION_PASTE_AFTER: {
			Ref<BTTask> sel = task_tree->get_selected();
			ERR_FAIL_COND(sel.is_null());
			EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Paste BT Tasks"));
			Ref<BTTask> parent = p_id == ACTION_PASTE_AFTER ? sel->get_parent() : sel;
			int idx = p_id == ACTION_PASTE_AFTER ? sel->get_index() + 1 : sel->get_child_count();
			Vector<Ref<BTTask>> newly_added;
			for (const Ref<BTTask> &clip : clipboard) {
				Ref<BTTask> dup = clip->clone();
				undo_redo->add_do_method(parent.ptr(), LW_NAME(add_child_at_index), dup, idx);
				undo_redo->add_undo_method(parent.ptr(), LW_NAME(remove_child), dup);
				newly_added.append(dup);
				idx += 1;
			}
			_commit_action_with_update(undo_redo);
			task_tree->clear_selection();
			for (const Ref<BTTask> &task : newly_added) {
				task_tree->add_selection(task);
			}
		} break;
		case ACTION_MOVE_UP: {
			Vector<Ref<BTTask>> selection = task_tree->get_selected_tasks();
			if (selection.is_empty()) {
				return;
			}
			for (const Ref<BTTask> &task : selection) {
				if (task->get_index() <= 0) {
					// If any selected is at the top of its hierarchy, we shouldn't move.
					return;
				}
			}
			EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Move BT Tasks"));
			for (int i = 0; i < selection.size(); i++) {
				Ref<BTTask> parent = selection[i]->get_parent();
				int idx = selection[i]->get_index();
				undo_redo->add_do_method(parent.ptr(), LW_NAME(remove_child), selection[i]);
				undo_redo->add_do_method(parent.ptr(), LW_NAME(add_child_at_index), selection[i], idx - 1);
			}
			for (int i = selection.size() - 1; i >= 0; i--) {
				Ref<BTTask> parent = selection[i]->get_parent();
				int idx = selection[i]->get_index();
				undo_redo->add_undo_method(parent.ptr(), LW_NAME(remove_child), selection[i]);
				undo_redo->add_undo_method(parent.ptr(), LW_NAME(add_child_at_index), selection[i], idx);
			}
			_commit_action_with_update(undo_redo);
		} break;
		case ACTION_MOVE_DOWN: {
			Vector<Ref<BTTask>> selection = task_tree->get_selected_tasks();
			if (selection.is_empty()) {
				return;
			}
			for (const Ref<BTTask> &task : selection) {
				if (task->get_parent().is_null() || task->get_index() >= task->get_parent()->get_child_count() - 1) {
					// If any selected is at the bottom of its hierarchy, we shouldn't move.
					return;
				}
			}
			EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Move BT Tasks"));
			for (int i = 0; i < selection.size(); i++) {
				Ref<BTTask> parent = selection[i]->get_parent();
				int idx = selection[i]->get_index();
				undo_redo->add_undo_method(parent.ptr(), LW_NAME(remove_child), selection[i]);
				undo_redo->add_undo_method(parent.ptr(), LW_NAME(add_child_at_index), selection[i], idx);
			}
			for (int i = selection.size() - 1; i >= 0; i--) {
				Ref<BTTask> parent = selection[i]->get_parent();
				int idx = selection[i]->get_index();
				undo_redo->add_do_method(parent.ptr(), LW_NAME(remove_child), selection[i]);
				undo_redo->add_do_method(parent.ptr(), LW_NAME(add_child_at_index), selection[i], idx + 1);
			}
			_commit_action_with_update(undo_redo);
		} break;
		case ACTION_DUPLICATE: {
			Vector<Ref<BTTask>> selection = task_tree->get_selected_tasks();
			if (selection.is_empty()) {
				return;
			}
			EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Duplicate BT Tasks"));
			Vector<Ref<BTTask>> duplicated;
			for (int i = selection.size() - 1; i >= 0; i--) {
				Ref<BTTask> parent = selection[i]->get_parent();
				if (parent.is_null()) {
					parent = selection[i];
				}
				const Ref<BTTask> &dup = selection[i]->clone();
				undo_redo->add_do_method(parent.ptr(), LW_NAME(add_child_at_index), dup, selection[i]->get_index() + 1);
				undo_redo->add_undo_method(parent.ptr(), LW_NAME(remove_child), dup);
				duplicated.append(dup);
			}
			_commit_action_with_update(undo_redo);
			task_tree->clear_selection();
			for (const Ref<BTTask> &dup : duplicated) {
				task_tree->add_selection(dup);
			}
		} break;
		case ACTION_MAKE_ROOT: {
			Ref<BTTask> task = task_tree->get_selected();
			if (task.is_valid() && task_tree->get_bt()->get_root_task() != task) {
				Ref<BTTask> parent = task->get_parent();
				ERR_FAIL_COND(parent.is_null());
				EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Make as BT Root"));
				undo_redo->add_do_method(parent.ptr(), LW_NAME(remove_child), task);
				Ref<BTTask> old_root = task_tree->get_bt()->get_root_task();
				undo_redo->add_do_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), task);
				undo_redo->add_do_method(task.ptr(), LW_NAME(add_child), old_root);
				undo_redo->add_undo_method(task.ptr(), LW_NAME(remove_child), old_root);
				undo_redo->add_undo_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), old_root);
				undo_redo->add_undo_method(parent.ptr(), LW_NAME(add_child_at_index), task, task->get_index());
				_commit_action_with_update(undo_redo);
			}
		} break;
		case ACTION_EXTRACT_SUBTREE: {
			Ref<BTTask> task = task_tree->get_selected();
			if (task.is_valid() && !IS_CLASS(task, BTSubtree)) {
				extract_dialog->popup_centered_ratio();
			}
		} break;
		case ACTION_CUT:
		case ACTION_REMOVE: {
			Vector<Ref<BTTask>> selection = task_tree->get_selected_tasks();
			if (selection.is_empty()) {
				return;
			}
			if (p_id == ACTION_CUT) {
				clipboard.clear();
			}
			EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Remove BT Tasks"), UndoRedo::MERGE_ALL);
			for (const Ref<BTTask> &task : selection) {
				if (task->is_root()) {
					undo_redo->add_do_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), Variant());
					undo_redo->add_undo_method(task_tree->get_bt().ptr(), LW_NAME(set_root_task), task_tree->get_bt()->get_root_task());
				} else {
					undo_redo->add_do_method(task->get_parent().ptr(), LW_NAME(remove_child), task);
					undo_redo->add_undo_method(task->get_parent().ptr(), LW_NAME(add_child_at_index), task, task->get_index());
				}
				if (p_id == ACTION_CUT) {
					clipboard.append(task->clone());
				}
			}
			_commit_action_with_update(undo_redo);
			EditorInterface::get_singleton()->edit_resource(task_tree->get_selected());
		} break;
	}
}

void LimboAIEditor::_on_probability_edited(double p_value) {
	Ref<BTTask> selected = task_tree->get_selected();
	ERR_FAIL_COND(selected.is_null());
	Ref<BTProbabilitySelector> probability_selector = selected->get_parent();
	ERR_FAIL_COND(probability_selector.is_null());
	if (percent_mode->is_pressed()) {
		probability_selector->set_probability(selected->get_index(), p_value * 0.01);
	} else {
		probability_selector->set_weight(selected->get_index(), p_value);
	}
}

void LimboAIEditor::_update_probability_edit() {
	Ref<BTTask> selected = task_tree->get_selected();
	ERR_FAIL_COND(selected.is_null());
	Ref<BTProbabilitySelector> prob = selected->get_parent();
	ERR_FAIL_COND(prob.is_null());
	double others_weight = prob->get_total_weight() - prob->get_weight(selected->get_index());
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
		case MISC_ONLINE_DOCUMENTATION: {
			LimboUtility::get_singleton()->open_doc_online();
		} break;
		case MISC_DOC_INTRODUCTION: {
			LimboUtility::get_singleton()->open_doc_introduction();
		} break;
		case MISC_DOC_CUSTOM_TASKS: {
			LimboUtility::get_singleton()->open_doc_custom_tasks();
		} break;
		case MISC_OPEN_DEBUGGER: {
			ERR_FAIL_COND(LimboDebuggerPlugin::get_singleton() == nullptr);
			if (LimboDebuggerPlugin::get_singleton()->get_first_session_window()->get_window_enabled()) {
				LimboDebuggerPlugin::get_singleton()->get_first_session_window()->set_window_enabled(true);
			} else {
#ifdef LIMBOAI_MODULE
				EditorNode::get_bottom_panel()->make_item_visible(EditorDebuggerNode::get_singleton());
				EditorDebuggerNode::get_singleton()->get_default_debugger()->switch_to_debugger(
						LimboDebuggerPlugin::get_singleton()->get_first_session_tab_index());
#elif LIMBOAI_GDEXTENSION
				// TODO: Unsure how to switch to debugger pane with GDExtension.
#endif
			}
		} break;
		case MISC_PROJECT_SETTINGS: {
			_edit_project_settings();
		} break;
		case MISC_LAYOUT_CLASSIC: {
			EDITOR_SETTINGS()->set_setting("limbo_ai/editor/layout", LAYOUT_CLASSIC);
			EDITOR_SETTINGS()->mark_setting_changed("limbo_ai/editor/layout");
			_update_banners();
		} break;
		case MISC_LAYOUT_WIDESCREEN_OPTIMIZED: {
			EDITOR_SETTINGS()->set_setting("limbo_ai/editor/layout", LAYOUT_WIDESCREEN_OPTIMIZED);
			EDITOR_SETTINGS()->mark_setting_changed("limbo_ai/editor/layout");
			_update_banners();
		} break;
		case MISC_CREATE_SCRIPT_TEMPLATE: {
			String template_path = _get_script_template_path();
			String template_dir = template_path.get_base_dir();

			if (!FILE_EXISTS(template_path)) {
				if (!DirAccess::dir_exists_absolute(template_dir)) {
					Error err = DirAccess::make_dir_recursive_absolute(template_dir);
					ERR_FAIL_COND(err != OK);
				}

				Ref<FileAccess> f = FileAccess::open(template_path, FileAccess::WRITE);
				ERR_FAIL_COND(f.is_null());

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
						"_TS_return SUCCESS\n"
						"\n\n"
						"# Strings returned from this method are displayed as warnings in the behavior tree editor (requires @tool).\n"
						"func _get_configuration_warnings() -> PackedStringArray:\n"
						"_TS_var warnings := PackedStringArray()\n"
						"_TS_return warnings\n";

				f->store_string(script_template);
				f->close();
			}

			EDITOR_FILE_SYSTEM()->scan();
			EDIT_SCRIPT(template_path);
		} break;
		case MISC_SEARCH_TREE: {
			task_tree->tree_search_show_and_focus();
		} break;
	}
}

void LimboAIEditor::_on_tree_task_selected(const Ref<BTTask> &p_task) {
	EditorInterface::get_singleton()->edit_resource(p_task);
}

void LimboAIEditor::_on_tree_task_activated() {
	Ref<BTTask> selected = task_tree->get_selected();
	ERR_FAIL_COND(selected.is_null());
	Ref<Script> scr = selected->get_script();
	if (scr.is_valid()) {
		EDIT_SCRIPT(scr->get_path());
	} else if (IS_CLASS(selected, BTSubtree)) {
		Ref<BehaviorTree> subtree = static_cast<Ref<BTSubtree>>(selected)->get_subtree();
		if (subtree.is_valid()) {
			EditorInterface::get_singleton()->edit_resource(subtree);
		} else {
			LimboUtility::get_singleton()->open_doc_class(selected->get_class());
		}
		EditorInterface::get_singleton()->edit_resource(subtree);
	} else {
		LimboUtility::get_singleton()->open_doc_class(selected->get_class());
	}
}

void LimboAIEditor::_on_visibility_changed() {
	if (task_tree->is_visible_in_tree()) {
		_switch_to_owner_scene_if_builtin(task_tree->get_bt());
		Ref<BTTask> sel = task_tree->get_selected();
		if (sel.is_valid()) {
			EditorInterface::get_singleton()->edit_resource(sel);
		} else if (task_tree->get_bt().is_valid() && EditorInterface::get_singleton()->get_inspector()->get_edited_object() != task_tree->get_bt().ptr()) {
			EditorInterface::get_singleton()->edit_resource(task_tree->get_bt());
		}

		task_palette->refresh();
		_update_banners();
	}
	_update_favorite_tasks();

	if (request_update_tabs && history.size() > 0) {
		_update_tabs();
		request_update_tabs = false;
	}
}

void LimboAIEditor::_on_save_pressed() {
	if (task_tree->get_bt().is_null()) {
		return;
	}
	String path = task_tree->get_bt()->get_path();
	if (path.is_empty()) {
		save_dialog->popup_centered_ratio();
	} else {
		_save_current_bt(path);
	}
}

void LimboAIEditor::_on_history_back() {
	ERR_FAIL_COND(history.size() == 0);
	idx_history = MAX(idx_history - 1, 0);
	EditorInterface::get_singleton()->edit_resource(history[idx_history]);
}

void LimboAIEditor::_on_history_forward() {
	ERR_FAIL_COND(history.size() == 0);
	idx_history = MIN(idx_history + 1, history.size() - 1);
	EditorInterface::get_singleton()->edit_resource(history[idx_history]);
}

void LimboAIEditor::_on_tasks_dragged(const TypedArray<BTTask> &p_tasks, Ref<BTTask> p_to_task, int p_to_pos) {
	ERR_FAIL_COND(p_to_task.is_null());
	if (p_tasks.is_empty()) {
		return;
	}

	// Remove descendants of selected.
	Vector<Ref<BTTask>> tasks_list;
	for (int i = 0; i < p_tasks.size(); i++) {
		Ref<BTTask> task = p_tasks[i];
		bool remove = false;
		for (int s_idx = 0; s_idx < p_tasks.size(); s_idx++) {
			Ref<BTTask> selected = p_tasks[s_idx];
			if (task->is_descendant_of(selected)) {
				remove = true;
				break;
			}
		}
		if (!remove) {
			tasks_list.push_back(task);
		}
	}

	EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Drag BT Task"));

	// Remove all tasks first so adding ordering is stable.
	int before_pos = 0;
	for (const Ref<BTTask> &task : tasks_list) {
		if (task->get_parent() == p_to_task && p_to_pos > task->get_index()) {
			before_pos += 1;
		}
		undo_redo->add_do_method(task->get_parent().ptr(), LW_NAME(remove_child), task);
	}

	for (int i = 0; i < tasks_list.size(); i++) {
		Ref<BTTask> task = tasks_list[i];
		undo_redo->add_do_method(p_to_task.ptr(), LW_NAME(add_child_at_index), task, p_to_pos + i - before_pos);
		undo_redo->add_undo_method(p_to_task.ptr(), LW_NAME(remove_child), task);
	}

	// Re-add tasks in later undo action so indexes match the old order.
	for (const Ref<BTTask> &task : tasks_list) {
		undo_redo->add_undo_method(task->get_parent().ptr(), LW_NAME(add_child_at_index), task, task->get_index());
	}

	_commit_action_with_update(undo_redo);
}

void LimboAIEditor::_on_resources_reload(const PackedStringArray &p_resources) {
	for (const String &res_path : p_resources) {
		if (!RESOURCE_IS_CACHED(res_path)) {
			continue;
		}

		if (RESOURCE_EXISTS(res_path, "BehaviorTree")) {
			Ref<BehaviorTree> res = RESOURCE_LOAD(res_path, "BehaviorTree");
			if (res.is_valid()) {
				if (history.has(res)) {
					disk_changed_files.insert(res_path);
				} else {
					Ref<BehaviorTree> reloaded = RESOURCE_LOAD_NO_CACHE(res_path, "BehaviorTree");
					res->copy_other(reloaded);
				}
			}
		}
	}

	// TODO: Find a way to allow resaving trees when they change outside of Godot.
	// * Currently, editor reloads them without asking in GDExtension. There is no Resource::editor_can_reload_from_file().
#ifdef LIMBOAI_MODULE
	if (disk_changed_files.size() > 0) {
		disk_changed_list->clear();
		disk_changed_list->set_hide_root(true);
		disk_changed_list->create_item();
		for (const String &fn : disk_changed_files) {
			TreeItem *ti = disk_changed_list->create_item();
			ti->set_text(0, fn);
		}

		if (!is_visible()) {
			EditorInterface::get_singleton()->set_main_screen_editor("LimboAI");
		}
		disk_changed->call_deferred("popup_centered_ratio", 0.5);
	}
#elif LIMBOAI_GDEXTENSION
	task_tree->update_tree();
#endif
}

void LimboAIEditor::_on_filesystem_changed() {
	if (history.size() == 0) {
		return;
	}

	if (is_visible_in_tree()) {
		_update_tabs();
	} else {
		request_update_tabs = true;
	}
}

void LimboAIEditor::_on_new_script_pressed() {
	PackedStringArray user_task_directories = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dirs");
	ERR_FAIL_INDEX_MSG(0, user_task_directories.size(), "LimboAI: No user task directory set");
	String default_task_dir = user_task_directories[0];
	EditorInterface::get_singleton()->get_script_editor()->open_script_create_dialog("BTAction", default_task_dir.path_join("new_task"));
}

void LimboAIEditor::_task_type_selected(const String &p_class_or_path) {
	change_type_popup->hide();

	Ref<BTTask> selected_task = task_tree->get_selected();
	ERR_FAIL_COND(selected_task.is_null());
	Ref<BTTask> new_task = _create_task_by_class_or_path(p_class_or_path);
	ERR_FAIL_COND_MSG(new_task.is_null(), "LimboAI: Unable to construct task.");

	EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Change BT task type"));
	undo_redo->add_do_method(this, LW_NAME(_replace_task), selected_task, new_task);
	undo_redo->add_undo_method(this, LW_NAME(_replace_task), new_task, selected_task);
	_commit_action_with_update(undo_redo);
}

void LimboAIEditor::_copy_version_info() {
	DisplayServer::get_singleton()->clipboard_set(version_btn->get_text());
}

void LimboAIEditor::_replace_task(const Ref<BTTask> &p_task, const Ref<BTTask> &p_by_task) {
	ERR_FAIL_COND(p_task.is_null());
	ERR_FAIL_COND(p_by_task.is_null());
	ERR_FAIL_COND(p_by_task->get_child_count() > 0);
	ERR_FAIL_COND(p_by_task->get_parent().is_valid());

	while (p_task->get_child_count() > 0) {
		Ref<BTTask> child = p_task->get_child(0);
		p_task->remove_child_at_index(0);
		p_by_task->add_child(child);
	}
	p_by_task->set_custom_name(p_task->get_custom_name());

	Ref<BTTask> parent = p_task->get_parent();
	if (parent.is_null()) {
		// Assuming root task is replaced.
		ERR_FAIL_COND(task_tree->get_bt().is_null());
		ERR_FAIL_COND(task_tree->get_bt()->get_root_task() != p_task);
		task_tree->get_bt()->set_root_task(p_by_task);
	} else {
		// Non-root task is replaced.
		int idx = p_task->get_index();

		double weight = 0.0;
		Ref<BTProbabilitySelector> probability_selector = parent;
		if (probability_selector.is_valid()) {
			weight = probability_selector->get_weight(idx);
		}

		parent->remove_child(p_task);
		parent->add_child_at_index(p_by_task, idx);

		if (probability_selector.is_valid()) {
			probability_selector->set_weight(idx, weight);
		}
	}
}

void LimboAIEditor::_tab_clicked(int p_tab) {
	if (updating_tabs) {
		return;
	}
	ERR_FAIL_INDEX(p_tab, history.size());
	EditorInterface::get_singleton()->edit_resource(history[p_tab]);
}

void LimboAIEditor::_tab_closed(int p_tab) {
	ERR_FAIL_INDEX(p_tab, history.size());
	Ref<BehaviorTree> history_bt = history[p_tab];
	if (history_bt.is_valid() && history_bt->is_connected(LW_NAME(changed), callable_mp(this, &LimboAIEditor::_set_as_dirty))) {
		history_bt->disconnect(LW_NAME(changed), callable_mp(this, &LimboAIEditor::_set_as_dirty));
	}
	if (tab_search_context.has(history_bt)) {
		tab_search_context.erase(history_bt);
	}

	history.remove_at(p_tab);
	idx_history = MIN(idx_history, history.size() - 1);
	TreeSearch::SearchInfo search_info_opened_tab;
	if (idx_history < 0) {
		_disable_editing();
	} else {
		EditorInterface::get_singleton()->edit_resource(history[idx_history]);
		ERR_FAIL_COND(!tab_search_context.has(history[idx_history]));
		search_info_opened_tab = tab_search_context[history[idx_history]];
	}

	task_tree->tree_search_set_search_info(search_info_opened_tab);
	_update_tabs();
}

void LimboAIEditor::_update_tabs() {
	updating_tabs = true;
	tab_bar->clear_tabs();

	Vector<String> short_names;
	// Keep track of how many times each short name is used.
	HashMap<String, int> usage_counts;

	for (int i = 0; i < history.size(); i++) {
		String tab_name;
		if (history[i]->get_path().contains("::")) {
			tab_name = history[i]->get_path().get_file();
		} else {
			tab_name = history[i]->get_path().get_file().get_basename();
		}
		short_names.append(tab_name);
		if (usage_counts.has(tab_name)) {
			usage_counts[tab_name] += 1;
		} else {
			usage_counts[tab_name] = 1;
		}
	}

	for (int i = 0; i < short_names.size(); i++) {
		String tab_name = short_names[i];
		if (tab_name.is_empty()) {
			tab_name = "[new]";
		} else if (usage_counts[tab_name] > 1) {
			// Use the full name if the short name is not unique.
			tab_name = history[i]->get_path().trim_prefix("res://");
		}
		tab_bar->add_tab(tab_name, LimboUtility::get_singleton()->get_task_icon("BehaviorTree"));
		if (i == idx_history) {
			tab_bar->set_tab_button_icon(tab_bar->get_tab_count() - 1, LimboUtility::get_singleton()->get_task_icon("LimboEditBlackboard"));
		}
	}

	if (idx_history >= 0) {
		ERR_FAIL_INDEX(idx_history, history.size());
		tab_bar->set_current_tab(idx_history);
	}

	updating_tabs = false;
}

void LimboAIEditor::_move_active_tab(int p_to_index) {
	ERR_FAIL_INDEX(p_to_index, history.size());
	if (idx_history == p_to_index) {
		return;
	}
	Ref<BehaviorTree> bt = history[idx_history];
	history.remove_at(idx_history);
	history.insert(p_to_index, bt);
	idx_history = p_to_index;
	_update_tabs();
}

void LimboAIEditor::_tab_input(const Ref<InputEvent> &p_input) {
	Ref<InputEventMouseButton> mb = p_input;
	if (mb.is_null()) {
		return;
	}
	int tab_idx = tab_bar->get_tab_idx_at_point(tab_bar->get_local_mouse_position());
	if (tab_idx < 0) {
		return;
	}
	if (mb->is_pressed() && mb->get_button_index() == LW_MBTN(MIDDLE)) {
		_tab_closed(tab_idx);
	} else if (mb->is_pressed() && mb->get_button_index() == LW_MBTN(RIGHT)) {
		_show_tab_context_menu();
	}
}

void LimboAIEditor::_show_tab_context_menu() {
	tab_menu->clear();
	tab_menu->add_shortcut(LW_GET_SHORTCUT("limbo_ai/jump_to_owner"), TabMenu::TAB_JUMP_TO_OWNER);
	tab_menu->add_item(TTR("Show in FileSystem"), TabMenu::TAB_SHOW_IN_FILESYSTEM);
	tab_menu->add_separator();
	tab_menu->add_shortcut(LW_GET_SHORTCUT("limbo_ai/close_tab"), TabMenu::TAB_CLOSE);
	tab_menu->add_item(TTR("Close Other Tabs"), TabMenu::TAB_CLOSE_OTHER);
	tab_menu->add_item(TTR("Close Tabs to the Right"), TabMenu::TAB_CLOSE_RIGHT);
	tab_menu->add_item(TTR("Close All Tabs"), TabMenu::TAB_CLOSE_ALL);
	tab_menu->set_position(get_screen_position() + get_local_mouse_position());
	tab_menu->reset_size();
	tab_menu->popup();
}

void LimboAIEditor::_tab_menu_option_selected(int p_id) {
	if (history.size() == 0) {
		// No tabs open, returning.
		return;
	}
	ERR_FAIL_INDEX(idx_history, history.size());

	switch (p_id) {
		case TAB_SHOW_IN_FILESYSTEM: {
			Ref<BehaviorTree> bt = history[idx_history];
			String path = bt->get_path();
			if (!path.is_empty()) {
				FileSystemDock *dock = EditorInterface::get_singleton()->get_file_system_dock();
				dock->navigate_to_path(path.get_slice("::", 0));
			}
		} break;
		case TAB_JUMP_TO_OWNER: {
			Ref<BehaviorTree> bt = history[idx_history];
			ERR_FAIL_COND(bt.is_null());
			String bt_path = bt->get_path();
			if (!bt_path.is_empty()) {
				owner_picker->pick_and_open_owner_of_resource(bt_path);
			}
		} break;
		case TAB_CLOSE: {
			_tab_closed(idx_history);
		} break;
		case TAB_CLOSE_OTHER: {
			Ref<BehaviorTree> bt = history[idx_history];
			history.clear();
			history.append(bt);
			idx_history = 0;
			_update_tabs();
		} break;
		case TAB_CLOSE_RIGHT: {
			for (int i = history.size() - 1; i > idx_history; i--) {
				history.remove_at(i);
			}
			_update_tabs();
		} break;
		case TAB_CLOSE_ALL: {
			history.clear();
			idx_history = -1;
			_disable_editing();
			_update_tabs();
		} break;
	}
}

void LimboAIEditor::_tab_plan_edited(int p_tab) {
	ERR_FAIL_INDEX(p_tab, history.size());
	if (history[p_tab]->get_blackboard_plan().is_valid()) {
		EditorInterface::get_singleton()->edit_resource(history[p_tab]->get_blackboard_plan());
	}
}

void LimboAIEditor::_reload_modified() {
	for (const String &res_path : disk_changed_files) {
		Ref<BehaviorTree> res = RESOURCE_LOAD(res_path, "BehaviorTree");
		if (res.is_valid()) {
			Ref<BehaviorTree> reloaded = RESOURCE_LOAD_NO_CACHE(res_path, "BehaviorTree");
			res->copy_other(reloaded);
			if (idx_history >= 0 && history.get(idx_history) == res) {
				edit_bt(res, true);
			}
		}
	}
	disk_changed_files.clear();
	task_tree->update_tree();
}

void LimboAIEditor::_resave_modified(String _str) {
	for (const String &res_path : disk_changed_files) {
		Ref<BehaviorTree> bt = RESOURCE_LOAD(res_path, "BehaviorTree");
		if (bt.is_valid()) {
			ERR_FAIL_COND(!bt->is_class("BehaviorTree"));
			if (RESOURCE_IS_EXTERNAL(bt)) {
				// Only resave external - scene files are handled by the editor.
				_save_bt(bt, bt->get_path());
			}
		}
	}
	task_tree->update_tree();
	disk_changed->hide();
	disk_changed_files.clear();
}

void LimboAIEditor::_popup_info_dialog(const String &p_text) {
	info_dialog->set_text(p_text);
	info_dialog->popup_centered();
}

void LimboAIEditor::_rename_task_confirmed() {
	ERR_FAIL_COND(!task_tree->get_selected().is_valid());
	rename_dialog->hide();

	EditorUndoRedoManager *undo_redo = _new_undo_redo_action(TTR("Set Custom Name"));
	undo_redo->add_do_method(task_tree->get_selected().ptr(), LW_NAME(set_custom_name), rename_edit->get_text());
	undo_redo->add_undo_method(task_tree->get_selected().ptr(), LW_NAME(set_custom_name), task_tree->get_selected()->get_custom_name());
	undo_redo->add_do_method(this, LW_NAME(_update_task_tree), task_tree->get_bt(), task_tree->get_selected());
	undo_redo->add_undo_method(this, LW_NAME(_update_task_tree), task_tree->get_bt(), task_tree->get_selected());
	undo_redo->commit_action();
}

void LimboAIEditor::save_all(bool p_external_only) {
	for (int i = 0; i < history.size(); i++) {
		Ref<BehaviorTree> bt = history.get(i);
		String path = bt->get_path();
		if (RESOURCE_EXISTS(path, "BehaviorTree") && (!p_external_only || RESOURCE_PATH_IS_EXTERNAL(path))) {
			_save_bt(bt, path);
		}
	}
}

void LimboAIEditor::_update_favorite_tasks() {
	for (int i = 0; i < fav_tasks_hbox->get_child_count(); i++) {
		fav_tasks_hbox->get_child(i)->queue_free();
	}
	Array favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
	for (int i = 0; i < favorite_tasks.size(); i++) {
		String task_meta = favorite_tasks[i];

		if (task_meta.is_empty() || (!FILE_EXISTS(task_meta) && !ClassDB::class_exists(task_meta))) {
			callable_mp(this, &LimboAIEditor::_update_banners).call_deferred();
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
		btn->set_meta(LW_NAME(task_meta), task_meta);
		btn->set_button_icon(LimboUtility::get_singleton()->get_task_icon(task_meta));
		btn->set_tooltip_text(vformat(TTR("Add %s task."), task_name));
		btn->set_flat(true);
		btn->add_theme_constant_override(LW_NAME(icon_max_width), 16 * EDSCALE); // Force user icons to be of the proper size.
		btn->set_focus_mode(Control::FOCUS_NONE);
		btn->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_add_task_by_class_or_path).bind(task_meta));
		fav_tasks_hbox->add_child(btn);
	}
}

void LimboAIEditor::_update_misc_menu() {
	PopupMenu *misc_menu = misc_btn->get_popup();

	misc_menu->clear();

	misc_menu->add_icon_item(theme_cache.doc_icon, TTR("Online Documentation"), MISC_ONLINE_DOCUMENTATION);
	misc_menu->add_icon_item(theme_cache.introduction_icon, TTR("Introduction"), MISC_DOC_INTRODUCTION);
	misc_menu->add_icon_item(theme_cache.introduction_icon, TTR("Creating custom tasks in GDScript"), MISC_DOC_CUSTOM_TASKS);

	misc_menu->add_separator();
#ifdef LIMBOAI_MODULE
	// * Disabled in GDExtension: Not sure how to switch to debugger pane.
	misc_menu->add_icon_shortcut(theme_cache.open_debugger_icon, LW_GET_SHORTCUT("limbo_ai/open_debugger"), MISC_OPEN_DEBUGGER);
#endif // LIMBOAI_MODULE
	misc_menu->add_item(TTR("Project Settings..."), MISC_PROJECT_SETTINGS);

	PopupMenu *layout_menu = Object::cast_to<PopupMenu>(misc_menu->get_node_or_null(NodePath("LayoutMenu")));
	if (layout_menu == nullptr) {
		layout_menu = memnew(PopupMenu);
		layout_menu->set_name("LayoutMenu");
		layout_menu->connect(LW_NAME(id_pressed), callable_mp(this, &LimboAIEditor::_misc_option_selected));
		misc_menu->add_child(layout_menu);
		layout_menu->add_radio_check_item(TTR("Classic"), MISC_LAYOUT_CLASSIC);
		layout_menu->add_radio_check_item(TTR("Widescreen Optimized"), MISC_LAYOUT_WIDESCREEN_OPTIMIZED);
	}
	misc_menu->add_submenu_item(TTR("Layout"), "LayoutMenu");
	EditorLayout saved_layout = (EditorLayout)(int)EDITOR_GET("limbo_ai/editor/layout");
	layout_menu->set_item_checked(0, saved_layout == LAYOUT_CLASSIC);
	layout_menu->set_item_checked(1, saved_layout == LAYOUT_WIDESCREEN_OPTIMIZED);

	misc_menu->add_separator();
	misc_menu->add_item(
			FILE_EXISTS(_get_script_template_path()) ? TTR("Edit Script Template") : TTR("Create Script Template"),
			MISC_CREATE_SCRIPT_TEMPLATE);

	misc_menu->add_separator();
	misc_menu->add_icon_shortcut(theme_cache.search_icon, LW_GET_SHORTCUT("limbo_ai/find_task"), MISC_SEARCH_TREE);
}

void LimboAIEditor::_update_banners() {
	for (int i = banners->get_child_count() - 1; i >= 0; i--) {
		if (banners->get_child(i)->has_meta(LW_NAME(managed))) {
			Node *banner = banners->get_child(i);
			banners->remove_child(banner);
			memfree(banner);
		}
	}

	PackedStringArray user_task_directories = GLOBAL_GET("limbo_ai/behavior_tree/user_task_dirs");
	for (const String &task_dir : user_task_directories) {
		if (!task_dir.is_empty() && !DirAccess::dir_exists_absolute(task_dir)) {
			ActionBanner *banner = memnew(ActionBanner);
			banner->set_text(vformat(TTR("Task folder not found: %s"), task_dir));
			banner->add_action(TTR("Create"), callable_mp(this, &LimboAIEditor::_create_user_task_dir).bind(task_dir), true);
			banner->add_action(TTR("Edit Path..."), callable_mp(this, &LimboAIEditor::_edit_project_settings));
			banner->add_spacer();
			banner->add_action(TTR("Help..."), callable_mp(LimboUtility::get_singleton(), &LimboUtility::open_doc_custom_tasks));
			banner->set_meta(LW_NAME(managed), Variant(true));
			banners->add_child(banner);
		}
	}

	Array favorite_tasks = GLOBAL_GET("limbo_ai/behavior_tree/favorite_tasks");
	for (int i = 0; i < favorite_tasks.size(); i++) {
		String task_meta = favorite_tasks[i];

		if (task_meta.is_empty() || (!FILE_EXISTS(task_meta) && !ClassDB::class_exists(task_meta))) {
			ActionBanner *banner = memnew(ActionBanner);
			banner->set_text(vformat(TTR("Favorite task not found: %s"), task_meta));
			banner->add_action(TTR("Remove"), callable_mp(this, &LimboAIEditor::_remove_task_from_favorite).bind(task_meta), true);
			banner->add_action(TTR("Edit Favorite Tasks..."), callable_mp(this, &LimboAIEditor::_edit_project_settings));
			banner->set_meta(LW_NAME(managed), Variant(true));
			banners->add_child(banner);
		}
	}

	EditorLayout saved_layout = (EditorLayout)(int)EDITOR_GET("limbo_ai/editor/layout");
	if (saved_layout != editor_layout) {
		ActionBanner *banner = memnew(ActionBanner);
		banner->set_text(TTR("Restart required to apply changes to editor layout"));
		banner->add_action(TTR("Save & Restart"), callable_mp(this, &LimboAIEditor::_save_and_restart), true);
		banner->set_meta(LW_NAME(managed), Variant(true));
		banners->add_child(banner);
	}
}

void LimboAIEditor::_do_update_theme_item_cache() {
	theme_cache.duplicate_task_icon = get_theme_icon(LW_NAME(Duplicate), LW_NAME(EditorIcons));
	theme_cache.edit_script_icon = get_theme_icon(LW_NAME(Script), LW_NAME(EditorIcons));
	theme_cache.make_root_icon = get_theme_icon(LW_NAME(NewRoot), LW_NAME(EditorIcons));
	theme_cache.move_task_down_icon = get_theme_icon(LW_NAME(MoveDown), LW_NAME(EditorIcons));
	theme_cache.move_task_up_icon = get_theme_icon(LW_NAME(MoveUp), LW_NAME(EditorIcons));
	theme_cache.open_debugger_icon = get_theme_icon(LW_NAME(Debug), LW_NAME(EditorIcons));
	theme_cache.doc_icon = get_theme_icon(LW_NAME(Help), LW_NAME(EditorIcons));
	theme_cache.introduction_icon = get_theme_icon(LW_NAME(Info), LW_NAME(EditorIcons));
	theme_cache.remove_task_icon = get_theme_icon(LW_NAME(Remove), LW_NAME(EditorIcons));
	theme_cache.rename_task_icon = get_theme_icon(LW_NAME(Rename), LW_NAME(EditorIcons));
	theme_cache.change_type_icon = get_theme_icon(LW_NAME(Reload), LW_NAME(EditorIcons));
	theme_cache.cut_icon = get_theme_icon(LW_NAME(ActionCut), LW_NAME(EditorIcons));
	theme_cache.copy_icon = get_theme_icon(LW_NAME(ActionCopy), LW_NAME(EditorIcons));
	theme_cache.paste_icon = get_theme_icon(LW_NAME(ActionPaste), LW_NAME(EditorIcons));
	theme_cache.search_icon = get_theme_icon(LW_NAME(Search), LW_NAME(EditorIcons));
	theme_cache.checked_icon = get_theme_icon("GuiChecked", LW_NAME(EditorIcons));
	theme_cache.unchecked_icon = get_theme_icon("GuiUnchecked", LW_NAME(EditorIcons));
	theme_cache.indeterminate_icon = get_theme_icon("GuiIndeterminate", LW_NAME(EditorIcons));

	theme_cache.behavior_tree_icon = LimboUtility::get_singleton()->get_task_icon("BehaviorTree");
	theme_cache.percent_icon = LimboUtility::get_singleton()->get_task_icon("LimboPercent");
	theme_cache.extract_subtree_icon = LimboUtility::get_singleton()->get_task_icon("LimboExtractSubtree");
}

void LimboAIEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_EXIT_TREE: {
			task_tree->unload();
			for (int i = 0; i < history.size(); i++) {
				if (history[i]->is_connected(LW_NAME(changed), callable_mp(this, &LimboAIEditor::_set_as_dirty))) {
					history[i]->disconnect(LW_NAME(changed), callable_mp(this, &LimboAIEditor::_set_as_dirty));
				}
			}
		} break;
		case NOTIFICATION_READY: {
			// **** Signals
			save_dialog->connect("file_selected", callable_mp(this, &LimboAIEditor::_save_current_bt));
			load_dialog->connect("file_selected", callable_mp(this, &LimboAIEditor::_load_bt));
			extract_dialog->connect("file_selected", callable_mp(this, &LimboAIEditor::_extract_subtree));
			new_btn->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_new_bt));
			load_btn->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_popup_file_dialog).bind(load_dialog));
			task_tree->connect("rmb_pressed", callable_mp(this, &LimboAIEditor::_on_tree_rmb));
			task_tree->connect("task_selected", callable_mp(this, &LimboAIEditor::_on_tree_task_selected));
			task_tree->connect("tasks_dragged", callable_mp(this, &LimboAIEditor::_on_tasks_dragged));
			task_tree->connect("task_activated", callable_mp(this, &LimboAIEditor::_on_tree_task_activated));
			task_tree->connect("probability_clicked", callable_mp(this, &LimboAIEditor::_action_selected).bind(ACTION_EDIT_PROBABILITY));
			task_tree->connect("visibility_changed", callable_mp(this, &LimboAIEditor::_on_visibility_changed));
			save_btn->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_on_save_pressed));
			misc_btn->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_update_misc_menu));
			misc_btn->get_popup()->connect("id_pressed", callable_mp(this, &LimboAIEditor::_misc_option_selected));
			task_palette->connect("task_selected", callable_mp(this, &LimboAIEditor::_add_task_by_class_or_path));
			task_palette->connect("favorite_tasks_changed", callable_mp(this, &LimboAIEditor::_update_favorite_tasks));
			change_type_palette->connect("task_selected", callable_mp(this, &LimboAIEditor::_task_type_selected));
			menu->connect("id_pressed", callable_mp(this, &LimboAIEditor::_action_selected));
			weight_mode->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_update_probability_edit));
			percent_mode->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_update_probability_edit));
			probability_edit->connect("value_changed", callable_mp(this, &LimboAIEditor::_on_probability_edited));
			probability_popup->connect("popup_hide", callable_mp(this, &LimboAIEditor::_probability_popup_closed));
			disk_changed->connect("confirmed", callable_mp(this, &LimboAIEditor::_reload_modified));
			disk_changed->connect("custom_action", callable_mp(this, &LimboAIEditor::_resave_modified));
			rename_dialog->connect("confirmed", callable_mp(this, &LimboAIEditor::_rename_task_confirmed));
			new_script_btn->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_on_new_script_pressed));
			tab_bar->connect("tab_clicked", callable_mp(this, &LimboAIEditor::_tab_clicked));
			tab_bar->connect("active_tab_rearranged", callable_mp(this, &LimboAIEditor::_move_active_tab));
			tab_bar->connect("tab_close_pressed", callable_mp(this, &LimboAIEditor::_tab_closed));
			tab_bar->connect(LW_NAME(gui_input), callable_mp(this, &LimboAIEditor::_tab_input));
			tab_menu->connect(LW_NAME(id_pressed), callable_mp(this, &LimboAIEditor::_tab_menu_option_selected));
			tab_bar->connect("tab_button_pressed", callable_mp(this, &LimboAIEditor::_tab_plan_edited));
			version_btn->connect(LW_NAME(pressed), callable_mp(this, &LimboAIEditor::_copy_version_info));

			EDITOR_FILE_SYSTEM()->connect("resources_reload", callable_mp(this, &LimboAIEditor::_on_resources_reload));
			EDITOR_FILE_SYSTEM()->connect("filesystem_changed", callable_mp(this, &LimboAIEditor::_on_filesystem_changed));
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_do_update_theme_item_cache();

			ADD_STYLEBOX_OVERRIDE(tab_bar_panel, "panel", get_theme_stylebox("tabbar_background", "TabContainer"));

			new_btn->set_button_icon(get_theme_icon(LW_NAME(New), LW_NAME(EditorIcons)));
			load_btn->set_button_icon(get_theme_icon(LW_NAME(Load), LW_NAME(EditorIcons)));
			save_btn->set_button_icon(get_theme_icon(LW_NAME(Save), LW_NAME(EditorIcons)));
			new_script_btn->set_button_icon(get_theme_icon(LW_NAME(ScriptCreate), LW_NAME(EditorIcons)));
			misc_btn->set_button_icon(get_theme_icon(LW_NAME(Tools), LW_NAME(EditorIcons)));

			_update_favorite_tasks();
		} break;
		case EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED: {
			if (is_visible_in_tree()) {
				_update_banners();
			}
		} break;
	}
}

void LimboAIEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_add_task", "task"), &LimboAIEditor::_add_task);
	ClassDB::bind_method(D_METHOD("_remove_task", "task"), &LimboAIEditor::_remove_task);
	ClassDB::bind_method(D_METHOD("_add_task_with_prototype", "prototype_task"), &LimboAIEditor::_add_task_with_prototype);
	ClassDB::bind_method(D_METHOD("_new_bt"), &LimboAIEditor::_new_bt);
	ClassDB::bind_method(D_METHOD("_save_bt", "path"), &LimboAIEditor::_save_current_bt);
	ClassDB::bind_method(D_METHOD("_load_bt", "path"), &LimboAIEditor::_load_bt);
	ClassDB::bind_method(D_METHOD("_update_task_tree", "bt", "specific_task"), &LimboAIEditor::_update_task_tree, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("edit_bt", "behavior_tree", "force_refresh"), &LimboAIEditor::edit_bt, Variant(false));
	ClassDB::bind_method(D_METHOD("_reload_modified"), &LimboAIEditor::_reload_modified);
	ClassDB::bind_method(D_METHOD("_resave_modified"), &LimboAIEditor::_resave_modified);
	ClassDB::bind_method(D_METHOD("_replace_task", "task", "by_task"), &LimboAIEditor::_replace_task);
	ClassDB::bind_method(D_METHOD("_popup_file_dialog"), &LimboAIEditor::_popup_file_dialog);
	ClassDB::bind_method(D_METHOD("get_edited_blackboard_plan"), &LimboAIEditor::get_edited_blackboard_plan);
}

LimboAIEditor::LimboAIEditor() {
	plugin = nullptr;
	idx_history = 0;
	dummy_history_context = memnew(Object);

	EDITOR_DEF("limbo_ai/editor/prefer_online_documentation", false);

	EDITOR_DEF("limbo_ai/editor/layout", 0);
#ifdef LIMBOAI_MODULE
	EDITOR_SETTINGS()->add_property_hint(PropertyInfo(Variant::INT, "limbo_ai/editor/layout", PROPERTY_HINT_ENUM, "Classic:0,Widescreen Optimized:1"));
	EDITOR_SETTINGS()->set_restart_if_changed("limbo_ai/editor/layout", true);
#elif LIMBOAI_GDEXTENSION
	PropertyInfo pinfo;
	pinfo.name = "limbo_ai/editor/layout";
	pinfo.type = Variant::INT;
	pinfo.hint = PROPERTY_HINT_ENUM;
	pinfo.hint_string = "Classic:0,Widescreen Optimized:1";
	EDITOR_SETTINGS()->add_property_info(pinfo);
#endif

	LW_SHORTCUT("limbo_ai/rename_task", TTR("Rename"), LW_KEY(F2));
	// Todo: Add override support for shortcuts.
	// LW_SHORTCUT_OVERRIDE("limbo_ai/rename_task", "macos", Key::ENTER);
	LW_SHORTCUT("limbo_ai/move_task_up", TTR("Move Up"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(UP)));
	LW_SHORTCUT("limbo_ai/move_task_down", TTR("Move Down"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(DOWN)));
	LW_SHORTCUT("limbo_ai/duplicate_task", TTR("Duplicate"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(D)));
	LW_SHORTCUT("limbo_ai/remove_task", TTR("Remove"), Key::KEY_DELETE);
	LW_SHORTCUT("limbo_ai/cut_task", TTR("Cut"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(X)));
	LW_SHORTCUT("limbo_ai/copy_task", TTR("Copy"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(C)));
	LW_SHORTCUT("limbo_ai/paste_task", TTR("Paste"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(V)));
	LW_SHORTCUT("limbo_ai/paste_task_after", TTR("Paste After Selected"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY_MASK(SHIFT) | LW_KEY(V)));

	LW_SHORTCUT("limbo_ai/new_behavior_tree", TTR("New Behavior Tree"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY_MASK(ALT) | LW_KEY(N)));
	LW_SHORTCUT("limbo_ai/save_behavior_tree", TTR("Save Behavior Tree"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY_MASK(ALT) | LW_KEY(S)));
	LW_SHORTCUT("limbo_ai/load_behavior_tree", TTR("Load Behavior Tree"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY_MASK(ALT) | LW_KEY(L)));
	LW_SHORTCUT("limbo_ai/open_debugger", TTR("Open Debugger"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY_MASK(ALT) | LW_KEY(D)));
	LW_SHORTCUT("limbo_ai/jump_to_owner", TTR("Jump to Owner"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(G)));
	LW_SHORTCUT("limbo_ai/close_tab", TTR("Close Tab"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(W)));
	LW_SHORTCUT("limbo_ai/find_task", TTR("Find Task"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(F)));
	LW_SHORTCUT("limbo_ai/hide_tree_search", TTR("Close Search"), (Key)(LW_KEY(ESCAPE)));

	// Intercept editor save scene action.
	LW_SHORTCUT("limbo_ai/editor_save_scene", TTR("Save Scene"), (Key)(LW_KEY_MASK(CMD_OR_CTRL) | LW_KEY(S)));

	set_process_shortcut_input(true);

	save_dialog = memnew(FileDialog);
	save_dialog->set_file_mode(FileDialog::FILE_MODE_SAVE_FILE);
	save_dialog->set_title(TTR("Save Behavior Tree"));
	save_dialog->add_filter("*.tres");
	save_dialog->hide();
	add_child(save_dialog);

	load_dialog = memnew(FileDialog);
	load_dialog->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
	load_dialog->set_title(TTR("Load Behavior Tree"));
	load_dialog->add_filter("*.tres");
	load_dialog->hide();
	add_child(load_dialog);

	extract_dialog = memnew(FileDialog);
	extract_dialog->set_file_mode(FileDialog::FILE_MODE_SAVE_FILE);
	extract_dialog->set_title(TTR("Save Extracted Tree"));
	extract_dialog->add_filter("*.tres");
	extract_dialog->hide();
	add_child(extract_dialog);

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
	new_btn->set_shortcut(LW_GET_SHORTCUT("limbo_ai/new_behavior_tree"));
	new_btn->set_flat(true);
	new_btn->set_focus_mode(Control::FOCUS_NONE);
	toolbar->add_child(new_btn);

	load_btn = memnew(Button);
	load_btn->set_text(TTR("Load"));
	load_btn->set_tooltip_text(TTR("Load behavior tree from a resource file."));
	load_btn->set_shortcut(LW_GET_SHORTCUT("limbo_ai/load_behavior_tree"));
	load_btn->set_flat(true);
	load_btn->set_focus_mode(Control::FOCUS_NONE);
	toolbar->add_child(load_btn);

	save_btn = memnew(Button);
	save_btn->set_text(TTR("Save"));
	save_btn->set_tooltip_text(TTR("Save edited behavior tree to a resource file."));
	save_btn->set_shortcut(LW_GET_SHORTCUT("limbo_ai/save_behavior_tree"));
	save_btn->set_flat(true);
	save_btn->set_focus_mode(Control::FOCUS_NONE);
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
	toolbar->add_child(misc_btn);

	HBoxContainer *version_hbox = memnew(HBoxContainer);
	version_hbox->set_h_size_flags(SIZE_EXPAND | SIZE_SHRINK_END);
	toolbar->add_child(version_hbox);

	TextureRect *logo = memnew(TextureRect);
	logo->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
	logo->set_texture(LimboUtility::get_singleton()->get_task_icon("LimboAI"));
	version_hbox->add_child(logo);

	version_btn = memnew(LinkButton);
	version_btn->set_text(TTR("v") + String(GET_LIMBOAI_FULL_VERSION()));
	version_btn->set_tooltip_text(TTR("Click to copy."));
	version_btn->set_self_modulate(Color(1, 1, 1, 0.65));
	version_btn->set_underline_mode(LinkButton::UNDERLINE_MODE_ON_HOVER);
	version_btn->set_v_size_flags(SIZE_SHRINK_CENTER);
	version_hbox->add_child(version_btn);

	Control *version_spacer = memnew(Control);
	version_spacer->set_custom_minimum_size(Size2(2, 0) * EDSCALE);
	version_hbox->add_child(version_spacer);

	tab_bar_panel = memnew(PanelContainer);
	vbox->add_child(tab_bar_panel);
	tab_bar_container = memnew(HBoxContainer);
	tab_bar_panel->add_child(tab_bar_container);

	tab_bar = memnew(TabBar);
	tab_bar->set_select_with_rmb(true);
	tab_bar->set_drag_to_rearrange_enabled(true);
	tab_bar->set_max_tab_width(int(EDITOR_GET("interface/scene_tabs/maximum_width")) * EDSCALE);
	tab_bar->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
	tab_bar->set_tab_close_display_policy(TabBar::CLOSE_BUTTON_SHOW_ACTIVE_ONLY);
	tab_bar->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	tab_bar->set_focus_mode(FocusMode::FOCUS_NONE);
	tab_bar_container->add_child(tab_bar);

	tab_menu = memnew(PopupMenu);
	add_child(tab_menu);

	owner_picker = memnew(OwnerPicker);
	add_child(owner_picker);

	hsc = memnew(HSplitContainer);
	hsc->set_h_size_flags(SIZE_EXPAND_FILL);
	hsc->set_v_size_flags(SIZE_EXPAND_FILL);
	hsc->set_focus_mode(FOCUS_NONE);
	vbox->add_child(hsc);

	task_tree = memnew(TaskTree);
	task_tree->set_v_size_flags(SIZE_EXPAND_FILL);
	task_tree->set_h_size_flags(SIZE_EXPAND_FILL);
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

	task_palette = memnew(TaskPalette());
	task_palette->hide();
	hsc->add_child(task_palette);

	banners = memnew(VBoxContainer);
	vbox->add_child(banners);

	editor_layout = (EditorLayout)(int)EDITOR_GET("limbo_ai/editor/layout");
	if (editor_layout == LAYOUT_WIDESCREEN_OPTIMIZED) {
		// * Alternative layout optimized for wide screen.
		VBoxContainer *sidebar_vbox = memnew(VBoxContainer);
		hsc->add_child(sidebar_vbox);
		sidebar_vbox->set_v_size_flags(SIZE_EXPAND_FILL);

		HBoxContainer *header_bar = memnew(HBoxContainer);
		sidebar_vbox->add_child(header_bar);
		Control *header_spacer = memnew(Control);
		header_bar->add_child(header_spacer);
		header_spacer->set_custom_minimum_size(Size2(6, 0) * EDSCALE);
		TextureRect *header_logo = Object::cast_to<TextureRect>(logo->duplicate());
		header_bar->add_child(header_logo);
		Label *header_title = memnew(Label);
		header_bar->add_child(header_title);
		header_title->set_text(TTR("Behavior Tree Editor"));
		header_title->set_v_size_flags(SIZE_SHRINK_CENTER);
		header_title->set_theme_type_variation("HeaderMedium");

		task_palette->reparent(sidebar_vbox);
		task_palette->set_v_size_flags(SIZE_EXPAND_FILL);

		VBoxContainer *editor_vbox = memnew(VBoxContainer);
		hsc->add_child(editor_vbox);
		toolbar->reparent(editor_vbox);
		tab_bar_panel->reparent(editor_vbox);
		task_tree->reparent(editor_vbox);
		usage_hint->reparent(editor_vbox);
		banners->reparent(editor_vbox);
	}

	hsc->set_split_offset((editor_layout == LAYOUT_CLASSIC ? -320 : 320) * EDSCALE);

	change_type_popup = memnew(PopupPanel);
	add_child(change_type_popup);
	{
		VBoxContainer *change_type_vbox = memnew(VBoxContainer);
		change_type_popup->add_child(change_type_vbox);

		Label *change_type_title = memnew(Label);
		change_type_vbox->add_child(change_type_title);
		change_type_title->set_theme_type_variation("HeaderSmall");
		change_type_title->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
		change_type_title->set_text(TTR("Choose New Type"));

		change_type_palette = memnew(TaskPalette);
		change_type_vbox->add_child(change_type_palette);
		change_type_palette->use_dialog_mode();
		change_type_palette->set_v_size_flags(SIZE_EXPAND_FILL);
	}

	menu = memnew(PopupMenu);
	add_child(menu);

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
		weight_mode->set_pressed_no_signal(true);

		percent_mode = memnew(Button);
		mode_hbox->add_child(percent_mode);
		percent_mode->set_toggle_mode(true);
		percent_mode->set_button_group(button_group);
		percent_mode->set_focus_mode(Control::FOCUS_NONE);
		percent_mode->set_text(TTR("Percent"));
		percent_mode->set_tooltip_text(TTR("Edit percent"));

		probability_edit = memnew(EditorSpinSlider);
		vbc->add_child(probability_edit);
		probability_edit->set_min(0.0);
		probability_edit->set_max(10.0);
		probability_edit->set_step(0.01);
		probability_edit->set_allow_greater(true);
		probability_edit->set_custom_minimum_size(Size2(200.0 * EDSCALE, 0.0));
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
		disk_changed->add_button(TTR("Resave"), !DisplayServer::get_singleton()->get_swap_cancel_ok(), "resave");
	}

	info_dialog = memnew(AcceptDialog);
	add_child(info_dialog);

	EditorInterface::get_singleton()->get_base_control()->add_child(disk_changed);

	GLOBAL_DEF(PropertyInfo(Variant::STRING, "limbo_ai/behavior_tree/behavior_tree_default_dir", PROPERTY_HINT_DIR), "res://ai/trees");
	PackedStringArray user_task_dir_default;
	user_task_dir_default.append("res://ai/tasks");
	GLOBAL_DEF(PropertyInfo(Variant::PACKED_STRING_ARRAY, "limbo_ai/behavior_tree/user_task_dirs", PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:", Variant::STRING, PROPERTY_HINT_DIR)), user_task_dir_default);

	String bt_default_dir = GLOBAL_GET("limbo_ai/behavior_tree/behavior_tree_default_dir");
	save_dialog->set_current_dir(bt_default_dir);
	load_dialog->set_current_dir(bt_default_dir);
	extract_dialog->set_current_dir(bt_default_dir);
}

LimboAIEditor::~LimboAIEditor() {
	memdelete(dummy_history_context);
}

//**** LimboAIEditor ^

//**** LimboAIEditorPlugin

void LimboAIEditorPlugin::_bind_methods() {
}

void LimboAIEditorPlugin::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			add_debugger_plugin(memnew(LimboDebuggerPlugin));
			add_inspector_plugin(memnew(EditorInspectorPluginBBPlan));

			EditorInspectorPluginVariableName *var_plugin = memnew(EditorInspectorPluginVariableName);
			var_plugin->set_editor_plan_provider(Callable(limbo_ai_editor, "get_edited_blackboard_plan"));
			add_inspector_plugin(var_plugin);

			EditorInspectorPluginPropertyPath *path_plugin = memnew(EditorInspectorPluginPropertyPath);
			add_inspector_plugin(path_plugin);

			EditorInspectorPluginBBParam *param_plugin = memnew(EditorInspectorPluginBBParam);
			param_plugin->set_plan_getter(Callable(limbo_ai_editor, "get_edited_blackboard_plan"));
			add_inspector_plugin(param_plugin);
		} break;
		case NOTIFICATION_ENTER_TREE: {
			// Add BehaviorTree to the list of resources that should open in a new inspector.
			PackedStringArray open_in_new_inspector = EDITOR_GET("interface/inspector/resources_to_open_in_new_inspector");
			if (!open_in_new_inspector.has("BehaviorTree")) {
				open_in_new_inspector.push_back("BehaviorTree");
				EDITOR_SETTINGS()->set_setting("interface/inspector/resources_to_open_in_new_inspector", open_in_new_inspector);
			}
		} break;
	}
}

#ifdef LIMBOAI_MODULE
void LimboAIEditorPlugin::make_visible(bool p_visible) {
#elif LIMBOAI_GDEXTENSION
void LimboAIEditorPlugin::_make_visible(bool p_visible) {
#endif
	limbo_ai_editor->set_visible(p_visible);
}

#ifdef LIMBOAI_MODULE
void LimboAIEditorPlugin::get_window_layout(Ref<ConfigFile> p_configuration) {
#elif LIMBOAI_GDEXTENSION
void LimboAIEditorPlugin::_get_window_layout(const Ref<ConfigFile> &p_configuration) {
#endif
	limbo_ai_editor->get_window_layout(p_configuration);
}

#ifdef LIMBOAI_MODULE
void LimboAIEditorPlugin::set_window_layout(Ref<ConfigFile> p_configuration) {
#elif LIMBOAI_GDEXTENSION
void LimboAIEditorPlugin::_set_window_layout(const Ref<ConfigFile> &p_configuration) {
#endif
	limbo_ai_editor->set_window_layout(p_configuration);
}

#ifdef LIMBOAI_MODULE
void LimboAIEditorPlugin::edit(Object *p_object) {
#elif LIMBOAI_GDEXTENSION
void LimboAIEditorPlugin::_edit(Object *p_object) {
#endif
	Ref<BehaviorTree> bt = Object::cast_to<BehaviorTree>(p_object);
	if (bt.is_valid()) {
		limbo_ai_editor->edit_bt(bt);
	}
}

#ifdef LIMBOAI_MODULE
bool LimboAIEditorPlugin::handles(Object *p_object) const {
#elif LIMBOAI_GDEXTENSION
bool LimboAIEditorPlugin::_handles(Object *p_object) const {
#endif
	if (Object::cast_to<BehaviorTree>(p_object)) {
		return true;
	}
	return false;
}

#ifdef LIMBOAI_MODULE
void LimboAIEditorPlugin::save_external_data() {
#elif LIMBOAI_GDEXTENSION
void LimboAIEditorPlugin::_save_external_data() {
#endif
	limbo_ai_editor->save_all(true);
}

#ifdef LIMBOAI_GDEXTENSION
Ref<Texture2D> LimboAIEditorPlugin::_get_plugin_icon() const {
	return LimboUtility::get_singleton()->get_task_icon("LimboAI");
}
#endif // LIMBOAI_GDEXTENSION

LimboAIEditorPlugin::LimboAIEditorPlugin() {
	limbo_ai_editor = memnew(LimboAIEditor());
	limbo_ai_editor->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	EditorInterface::get_singleton()->get_editor_main_screen()->add_child(limbo_ai_editor);
	limbo_ai_editor->hide();
	limbo_ai_editor->set_plugin(this);
}

LimboAIEditorPlugin::~LimboAIEditorPlugin() {
}

#endif // ! TOOLS_ENABLED
