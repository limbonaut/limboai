/* limbo_debugger_plugin.h */

#ifndef LIMBO_DEBUGGER_PLUGIN_H
#define LIMBO_DEBUGGER_PLUGIN_H

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/typedefs.h"
#include "editor/plugins/editor_debugger_plugin.h"
#include "modules/limboai/debugger/behavior_tree_view.h"
#include "scene/gui/box_container.h"
#include "scene/gui/item_list.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/texture_rect.h"

class LimboDebuggerTab : public PanelContainer {
	GDCLASS(LimboDebuggerTab, PanelContainer);

private:
	Ref<EditorDebuggerSession> session;
	HSplitContainer *hsc;
	Label *message;
	ItemList *bt_list;
	BehaviorTreeView *bt_view;
	VBoxContainer *view_box;
	HBoxContainer *info_box;
	TextureRect *info_icon;
	Label *info_message;

	void _set_info_message(const String &p_message);
	void _bt_selected(int p_idx);

public:
	void start_session();
	void stop_session();
	void update_bt_list(const Array &p_items);
	BehaviorTreeView *get_behavior_tree_view() const { return bt_view; }

	LimboDebuggerTab(Ref<EditorDebuggerSession> p_session);
};

class LimboDebuggerPlugin : public EditorDebuggerPlugin {
	GDCLASS(LimboDebuggerPlugin, EditorDebuggerPlugin);

private:
	LimboDebuggerTab *tab;

public:
	void setup_session(int p_idx) override;
	bool has_capture(const String &p_capture) const override;
	bool capture(const String &p_message, const Array &p_data, int p_session) override;

	LimboDebuggerPlugin();
};

#endif // LIMBO_DEBUGGER_PLUGIN