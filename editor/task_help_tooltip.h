/**
 * task_help_tooltip.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#ifndef TASK_HELP_TOOLTIP_H
#define TASK_HELP_TOOLTIP_H

#ifdef LIMBOAI_GDEXTENSION

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/popup_panel.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

using namespace godot;

// Replicates EditorHelpBitTooltip behavior for GDExtension.
// This is a popup-based tooltip that allows hovering over it (unlike standard tooltips).
// The tooltip auto-closes when the mouse leaves both the target and the tooltip itself.
class TaskHelpTooltip : public PopupPanel {
	GDCLASS(TaskHelpTooltip, PopupPanel);

private:
	static bool _is_tooltip_visible;

	Timer *timer = nullptr;
	uint64_t _enter_tree_time = 0;
	bool _is_mouse_inside_tooltip = false;

	VBoxContainer *vbox = nullptr;
	RichTextLabel *title = nullptr;
	RichTextLabel *content = nullptr;

	float content_min_height = 0.0;
	float content_max_height = 0.0;

	static Control *_make_invisible_control();

	void _start_timer();
	void _target_gui_input(const Ref<InputEvent> &p_event);

	void _update_content_height();
	void _update_theme();
	void _add_text_to_rt(const String &p_bbcode, RichTextLabel *p_rt);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Creates a tooltip for a given class or script path.
	// Returns an invisible control to replace the standard tooltip.
	// p_target: The control that triggered the tooltip.
	// p_class_or_script_path: The class name or script path (starting with "res://").
	// p_description: Pre-fetched description text (can be empty, will be fetched if empty).
	[[nodiscard]] static Control *make_tooltip(Control *p_target, const String &p_class_or_script_path, const String &p_description = String());

	void set_content(const String &p_class_name, const String &p_description);

	void popup_under_cursor();

	TaskHelpTooltip(Control *p_target);
	TaskHelpTooltip();
};

#endif // LIMBOAI_GDEXTENSION

#endif // TASK_HELP_TOOLTIP_H

#endif // TOOLS_ENABLED