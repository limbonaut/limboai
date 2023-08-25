/**
 * limbo_debugger_plugin.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#ifndef LIMBO_DEBUGGER_PLUGIN_H
#define LIMBO_DEBUGGER_PLUGIN_H

#include "modules/limboai/editor/debugger/behavior_tree_data.h"
#include "modules/limboai/editor/debugger/behavior_tree_view.h"

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/typedefs.h"
#include "editor/plugins/editor_debugger_plugin.h"
#include "editor/window_wrapper.h"
#include "scene/gui/box_container.h"
#include "scene/gui/item_list.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/texture_rect.h"

class LimboDebuggerTab : public PanelContainer {
	GDCLASS(LimboDebuggerTab, PanelContainer);

private:
	List<String> active_bt_players;
	Ref<EditorDebuggerSession> session;
	VBoxContainer *root_vb = nullptr;
	HBoxContainer *toolbar = nullptr;
	HSplitContainer *hsc = nullptr;
	Label *info_message = nullptr;
	ItemList *bt_player_list = nullptr;
	BehaviorTreeView *bt_view = nullptr;
	VBoxContainer *view_box = nullptr;
	HBoxContainer *alert_box = nullptr;
	TextureRect *alert_icon = nullptr;
	Label *alert_message = nullptr;
	LineEdit *filter_players = nullptr;

	Button *make_floating = nullptr;
	WindowWrapper *window_wrapper = nullptr;

	void _show_alert(const String &p_message);
	void _update_bt_player_list(const List<String> &p_node_paths, const String &p_filter);
	void _bt_selected(int p_idx);
	void _filter_changed(String p_text);
	void _window_visibility_changed(bool p_visible);

public:
	void start_session();
	void stop_session();
	void update_active_bt_players(const Array &p_node_paths);
	BehaviorTreeView *get_behavior_tree_view() const { return bt_view; }
	String get_selected_bt_player();
	void update_behavior_tree(const BehaviorTreeData &p_data);

	LimboDebuggerTab(Ref<EditorDebuggerSession> p_session, WindowWrapper *p_wrapper);
};

class LimboDebuggerPlugin : public EditorDebuggerPlugin {
	GDCLASS(LimboDebuggerPlugin, EditorDebuggerPlugin);

private:
	static LimboDebuggerPlugin *singleton;

	LimboDebuggerTab *tab = nullptr;
	WindowWrapper *window_wrapper = nullptr;

	void _window_visibility_changed(bool p_visible);

public:
	static _FORCE_INLINE_ LimboDebuggerPlugin *get_singleton() { return singleton; }

	void setup_session(int p_idx) override;
	bool has_capture(const String &p_capture) const override;
	bool capture(const String &p_message, const Array &p_data, int p_session) override;

	WindowWrapper *get_session_tab() const;
	int get_session_tab_index() const;

	LimboDebuggerPlugin();
	~LimboDebuggerPlugin();
};

#endif // LIMBO_DEBUGGER_PLUGIN

#endif // TOOLS_ENABLED