/* limbo_debugger_view.h */

#include "limbo_debugger_plugin.h"
#include "core/debugger/engine_debugger.h"
#include "core/math/math_defs.h"
#include "core/object/callable_method_pointer.h"
#include "core/os/memory.h"
#include "core/string/print_string.h"
#include "core/variant/array.h"
#include "editor/editor_scale.h"
#include "editor/plugins/editor_debugger_plugin.h"
#include "limbo_debugger.h"
#include "modules/limboai/debugger/behavior_tree_data.h"
#include "modules/limboai/debugger/behavior_tree_view.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/split_container.h"
#include "scene/gui/texture_rect.h"

/////////////////////// LimboDebuggerTab

void LimboDebuggerTab::start_session() {
	bt_list->clear();
	bt_view->clear();
	info_box->hide();
	hsc->show();
	message->hide();
	session->send_message("limboai:start_session", Array());
}

void LimboDebuggerTab::stop_session() {
	hsc->hide();
	message->show();
	session->send_message("limboai:stop_session", Array());
}

void LimboDebuggerTab::update_bt_list(const Array &p_node_paths) {
	// Remember selected item.
	String selected_bt = "";
	if (bt_list->is_anything_selected()) {
		selected_bt = bt_list->get_item_text(bt_list->get_selected_items().get(0));
	}

	bt_list->clear();
	int select_idx = -1;
	for (int i = 0; i < p_node_paths.size(); i++) {
		bt_list->add_item(p_node_paths[i]);
		// Make item text shortened from the left, e.g ".../Agent/BTPlayer".
		bt_list->set_item_text_direction(i, TEXT_DIRECTION_RTL);
		if (p_node_paths[i] == selected_bt) {
			select_idx = i;
		}
	}

	// Restore selected item.
	if (select_idx > -1) {
		bt_list->select(select_idx);
	} else if (!selected_bt.is_empty()) {
		_set_info_message(TTR("Node instance is gone"));
	}
}

void LimboDebuggerTab::_set_info_message(const String &p_message) {
	info_message->set_text(p_message);
	info_icon->set_texture(get_theme_icon(SNAME("NodeInfo"), SNAME("EditorIcons")));
	info_box->set_visible(!p_message.is_empty());
}

void LimboDebuggerTab::_bt_selected(int p_idx) {
	info_box->hide();
	bt_view->clear();
	NodePath path = bt_list->get_item_text(p_idx);
	Array data;
	data.push_back(path);
	session->send_message("limboai:track_bt_updates", data);
}

LimboDebuggerTab::LimboDebuggerTab(Ref<EditorDebuggerSession> p_session) {
	session = p_session;

	hsc = memnew(HSplitContainer);
	add_child(hsc);

	bt_list = memnew(ItemList);
	hsc->add_child(bt_list);
	bt_list->set_custom_minimum_size(Size2(240.0 * EDSCALE, 0.0));
	bt_list->connect(SNAME("item_selected"), callable_mp(this, &LimboDebuggerTab::_bt_selected));

	view_box = memnew(VBoxContainer);
	{
		hsc->add_child(view_box);

		bt_view = memnew(BehaviorTreeView);
		view_box->add_child(bt_view);
		bt_view->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		bt_view->set_v_size_flags(Control::SIZE_EXPAND_FILL);

		info_box = memnew(HBoxContainer);
		view_box->add_child(info_box);
		info_box->hide();

		info_icon = memnew(TextureRect);
		info_box->add_child(info_icon);
		info_icon->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);

		info_message = memnew(Label);
		info_box->add_child(info_message);
		info_message->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	}

	message = memnew(Label);
	add_child(message);
	message->set_text(TTR("Run project to start debugging"));
	message->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	message->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	message->set_anchors_preset(Control::PRESET_CENTER);

	stop_session();
}

//////////////////////// LimboDebuggerPlugin

void LimboDebuggerPlugin::setup_session(int p_idx) {
	Ref<EditorDebuggerSession> session = get_session(p_idx);
	tab = memnew(LimboDebuggerTab(session));
	tab->set_name("LimboAI");
	session->connect(SNAME("started"), callable_mp(tab, &LimboDebuggerTab::start_session));
	session->connect(SNAME("stopped"), callable_mp(tab, &LimboDebuggerTab::stop_session));
	session->add_session_tab(tab);
}

bool LimboDebuggerPlugin::capture(const String &p_message, const Array &p_data, int p_session) {
	bool captured = true;
	if (p_message == "limboai:active_behavior_trees") {
		tab->update_bt_list(p_data);
	} else if (p_message == "limboai:bt_update") {
		BehaviorTreeData data = BehaviorTreeData();
		data.deserialize(p_data);
		tab->get_behavior_tree_view()->update_tree(data);
	} else {
		captured = false;
	}
	return captured;
}

bool LimboDebuggerPlugin::has_capture(const String &p_capture) const {
	return p_capture == "limboai";
}

LimboDebuggerPlugin::LimboDebuggerPlugin() {
	tab = nullptr;
}