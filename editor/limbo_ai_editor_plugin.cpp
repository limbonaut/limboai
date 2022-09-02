/* limbo_ai_editor_plugin.cpp */

#ifdef TOOLS_ENABLED

#include "limbo_ai_editor_plugin.h"

#include "../bt/composites/bt_parallel.h"
#include "../bt/composites/bt_selector.h"
#include "../bt/composites/bt_sequence.h"
#include "core/class_db.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/math/math_defs.h"
#include "core/object.h"
#include "core/os/memory.h"
#include "core/print_string.h"
#include "core/string_name.h"
#include "core/variant.h"
#include "core/vector.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
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
	p_item->set_icon(0, task->get_icon());
	p_item->set_editable(0, false);

	// TODO: Update configuration warning.

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
	}
	last_selected = get_selected();
	emit_signal("task_selected", last_selected);
}

void TaskTree::load_bt(const Ref<BehaviorTree> &p_behavior_tree) {
	ERR_FAIL_COND_MSG(p_behavior_tree.is_null(), "Tried to load a null tree.");
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

void TaskTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_item_rmb_selected"), &TaskTree::_on_item_rmb_selected);
	ClassDB::bind_method(D_METHOD("_on_item_selected"), &TaskTree::_on_item_selected);
	ClassDB::bind_method(D_METHOD("load_bt", "p_behavior_tree"), &TaskTree::load_bt);
	ClassDB::bind_method(D_METHOD("get_bt"), &TaskTree::get_bt);
	ClassDB::bind_method(D_METHOD("update_tree"), &TaskTree::update_tree);
	ClassDB::bind_method(D_METHOD("update_task", "p_task"), &TaskTree::update_task);
	ClassDB::bind_method(D_METHOD("get_selected"), &TaskTree::get_selected);
	ClassDB::bind_method(D_METHOD("deselect"), &TaskTree::deselect);

	ADD_SIGNAL(MethodInfo("rmb_pressed"));
	ADD_SIGNAL(MethodInfo("task_selected"));
}

TaskTree::TaskTree() {
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
}

TaskTree::~TaskTree() {
}

////////////////////////////////////////////////////////////////////////////////

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

void TaskSection::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_task_button_pressed", "p_class"), &TaskSection::_on_task_button_pressed);
	ClassDB::bind_method(D_METHOD("_on_header_pressed"), &TaskSection::_on_header_pressed);

	ADD_SIGNAL(MethodInfo("task_button_pressed"));
}

TaskSection::TaskSection(const StringName &p_class_or_resource, const StringName &p_section, EditorNode *p_editor) {
	section_header = memnew(Button);
	add_child(section_header);
	section_header->set_text(p_section);
	section_header->set_icon(p_editor->get_gui_base()->get_icon("GuiTreeArrowDown", "EditorIcons"));
	section_header->set_focus_mode(FOCUS_NONE);
	section_header->connect("pressed", this, "_on_header_pressed");

	tasks_container = memnew(HFlowContainer);
	add_child(tasks_container);

	List<StringName> composites;
	ClassDB::get_inheriters_from_class(p_class_or_resource, &composites);
	for (List<StringName>::Element *cn = composites.front(); cn; cn = cn->next()) {
		Button *task_btn = memnew(Button);
		task_btn->set_text(cn->get());
		task_btn->set_icon(p_editor->get_class_icon(cn->get()));
		task_btn->set_focus_mode(FOCUS_NONE);
		task_btn->set_h_size_flags(SIZE_EXPAND_FILL);
		task_btn->connect("pressed", this, "_on_task_button_pressed", varray(cn->get()));
		tasks_container->add_child(task_btn);
	}

	if (tasks_container->get_child_count() == 0) {
		hide();
	}
}

TaskSection::~TaskSection() {
}

////////////////////////////////////////////////////////////////////////////////

void TaskPanel::_on_task_button_pressed(const StringName &p_task) {
	emit_signal("task_selected", p_task);
}

void TaskPanel::_on_filter_text_changed(String p_text) {
	for (int i = 0; i < sections->get_child_count(); i++) {
		TaskSection *sec = Object::cast_to<TaskSection>(sections->get_child(i));
		sec->set_filter(p_text);
	}
}

void TaskPanel::_init() {
	filter_edit->set_right_icon(get_icon("Search", "EditorIcons"));
}

void TaskPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_init"), &TaskPanel::_init);
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

	TaskSection *comp_sec = memnew(TaskSection("BTComposite", "Composites", p_editor));
	sections->add_child(comp_sec);
	comp_sec->connect("task_button_pressed", this, "_on_task_button_pressed");

	TaskSection *dec_sec = memnew(TaskSection("BTDecorator", "Decorators", p_editor));
	sections->add_child(dec_sec);
	dec_sec->connect("task_button_pressed", this, "_on_task_button_pressed");

	TaskSection *act_sec = memnew(TaskSection("BTAction", "Actions", p_editor));
	sections->add_child(act_sec);
	act_sec->connect("task_button_pressed", this, "_on_task_button_pressed");

	TaskSection *cond_sec = memnew(TaskSection("BTCondition", "Conditions", p_editor));
	sections->add_child(cond_sec);
	cond_sec->connect("task_button_pressed", this, "_on_task_button_pressed");

	call_deferred("_init");
}

TaskPanel::~TaskPanel() {
}

////////////////////////////////////////////////////////////////////////////////

void LimboAIEditor::_add_task(const Ref<BTTask> &p_prototype) {
	ERR_FAIL_COND(p_prototype.is_null());
	Ref<BTTask> parent = task_tree->get_selected();
	if (parent.is_null()) {
		parent = task_tree->get_bt()->get_root_task();
	}
	if (parent.is_null()) {
		task_tree->get_bt()->set_root_task(p_prototype->clone());
	} else {
		parent->add_child(p_prototype->clone());
	}
	task_tree->update_tree();
}

void LimboAIEditor::_update_header() {
	String text = task_tree->get_bt()->get_path();
	if (text.empty()) {
		text = TTR("New Behavior Tree");
	}
	header->set_text(text);
	header->set_icon(editor->get_object_icon(task_tree->get_bt().ptr(), "BehaviorTree"));
}

void LimboAIEditor::_new_bt() {
	BehaviorTree *bt = memnew(BehaviorTree);
	bt->set_root_task(memnew(BTSelector));
	task_tree->load_bt(bt);
	_update_header();
}

void LimboAIEditor::_save_bt(String p_path) {
	ERR_FAIL_COND_MSG(p_path.empty(), "Empty p_path");
	ERR_FAIL_COND_MSG(task_tree->get_bt().is_null(), "Behavior Tree is null.");
	task_tree->get_bt()->set_path(p_path, true);
	ResourceSaver::save(p_path, task_tree->get_bt(), ResourceSaver::FLAG_CHANGE_PATH);
	_update_header();
}

void LimboAIEditor::_load_bt(String p_path) {
	ERR_FAIL_COND_MSG(p_path.empty(), "Empty p_path");
	task_tree->load_bt(ResourceLoader::load(p_path, "BehaviorTree"));
	_update_header();
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
				parent->add_child(sel->clone());
				task_tree->update_tree();
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
			}
		} break;
	}
}

void LimboAIEditor::_on_tree_task_selected(const Ref<BTTask> &p_task) const {
	editor->edit_resource(p_task);
}

void LimboAIEditor::_on_panel_task_selected(const StringName &p_task) {
	_add_task(Ref<BTTask>(ClassDB::instance(p_task)));
}

void LimboAIEditor::_on_visibility_changed() const {
	if (is_visible()) {
		Ref<BTTask> sel = task_tree->get_selected();
		if (sel.is_valid()) {
			editor->edit_resource(sel);
		} else {
			editor->edit_resource(task_tree->get_bt());
		}
	}
}

void LimboAIEditor::_on_header_pressed() const {
	task_tree->deselect();
	editor->edit_resource(task_tree->get_bt());
}

void LimboAIEditor::_on_save_pressed() {
	String path = task_tree->get_bt()->get_path();
	if (path.empty()) {
		save_dialog->popup_centered_ratio();
	} else {
		_save_bt(path);
	}
}

void LimboAIEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_add_task", "p_task"), &LimboAIEditor::_add_task);
	ClassDB::bind_method(D_METHOD("_on_tree_rmb"), &LimboAIEditor::_on_tree_rmb);
	ClassDB::bind_method(D_METHOD("_on_action_selected", "p_id"), &LimboAIEditor::_on_action_selected);
	ClassDB::bind_method(D_METHOD("_on_tree_task_selected", "p_task"), &LimboAIEditor::_on_tree_task_selected);
	ClassDB::bind_method(D_METHOD("_on_panel_task_selected", "p_task"), &LimboAIEditor::_on_panel_task_selected);
	ClassDB::bind_method(D_METHOD("_on_visibility_changed"), &LimboAIEditor::_on_visibility_changed);
	ClassDB::bind_method(D_METHOD("_on_header_pressed"), &LimboAIEditor::_on_header_pressed);
	ClassDB::bind_method(D_METHOD("_on_save_pressed"), &LimboAIEditor::_on_save_pressed);
	ClassDB::bind_method(D_METHOD("_new_bt"), &LimboAIEditor::_new_bt);
	ClassDB::bind_method(D_METHOD("_save_bt", "p_path"), &LimboAIEditor::_save_bt);
	ClassDB::bind_method(D_METHOD("_load_bt", "p_path"), &LimboAIEditor::_load_bt);
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
	selector_btn->connect("pressed", this, "_add_task", varray(Ref<BTTask>(memnew(BTSelector))));
	panel->add_child(selector_btn);

	Button *sequence_btn = memnew(Button);
	sequence_btn->set_text(TTR("Sequence"));
	sequence_btn->set_tooltip(TTR("Add Sequence task."));
	sequence_btn->set_icon(editor->get_class_icon("BTSequence"));
	sequence_btn->set_flat(true);
	sequence_btn->set_focus_mode(Control::FOCUS_NONE);
	sequence_btn->connect("pressed", this, "_add_task", varray(Ref<BTTask>(memnew(BTSequence))));
	panel->add_child(sequence_btn);

	Button *parallel_btn = memnew(Button);
	parallel_btn->set_text(TTR("Parallel"));
	parallel_btn->set_tooltip(TTR("Add Parallel task."));
	parallel_btn->set_icon(editor->get_class_icon("BTParallel"));
	parallel_btn->set_flat(true);
	parallel_btn->set_focus_mode(Control::FOCUS_NONE);
	parallel_btn->connect("pressed", this, "_add_task", varray(Ref<BTTask>(memnew(BTParallel))));
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

	header = memnew(Button);
	vb->add_child(header);
	header->set_text_align(Button::ALIGN_LEFT);
	header->add_constant_override("hseparation", 8);
	header->connect("pressed", this, "_on_header_pressed");

	HSplitContainer *hsc = memnew(HSplitContainer);
	vb->add_child(hsc);
	hsc->set_h_size_flags(SIZE_EXPAND_FILL);
	hsc->set_v_size_flags(SIZE_EXPAND_FILL);

	task_tree = memnew(TaskTree);
	hsc->add_child(task_tree);
	task_tree->set_v_size_flags(SIZE_EXPAND_FILL);
	task_tree->set_h_size_flags(SIZE_EXPAND_FILL);
	task_tree->connect("rmb_pressed", this, "_on_tree_rmb");
	task_tree->connect("task_selected", this, "_on_tree_task_selected");

	TaskPanel *task_panel = memnew(TaskPanel(p_editor));
	hsc->add_child(task_panel);
	hsc->set_split_offset(-400);
	task_panel->connect("task_selected", this, "_on_panel_task_selected");

	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", this, "_on_action_selected");
	menu->set_hide_on_window_lose_focus(true);

	BehaviorTree *bt = memnew(BehaviorTree);
	BTSelector *seq = memnew(BTSelector);
	bt->set_root_task(seq);

	task_tree->load_bt(bt);
	_update_header();

	task_tree->connect("visibility_changed", this, "_on_visibility_changed");
}

LimboAIEditor::~LimboAIEditor() {
}

////////////////////////////////////////////////////////////////////////////////

const Ref<Texture> LimboAIEditorPlugin::get_icon() const {
	// TODO:
	return nullptr;
}

void LimboAIEditorPlugin::_notification(int p_notification) {
	// print_line(vformat("NOTIFICATION: %d", p_notification));
}

void LimboAIEditorPlugin::make_visible(bool p_visible) {
	limbo_ai_editor->set_visible(p_visible);
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