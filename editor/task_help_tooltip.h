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

#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>

using namespace godot;

// Custom tooltip control for displaying task documentation in GDExtension.
// This is a simplified version of EditorHelpBitTooltip, which is not exposed in the Godot API.
class TaskHelpTooltip : public PanelContainer {
	GDCLASS(TaskHelpTooltip, PanelContainer);

private:
	RichTextLabel *rich_text = nullptr;
	String class_name;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Create a tooltip for a given class or script path.
	// p_class_or_script_path: The class name or script path (starting with "res://").
	// p_description: Pre-fetched description text (can be empty, will be fetched if empty).
	static TaskHelpTooltip *make_tooltip(const String &p_class_or_script_path, const String &p_description = String());

	void set_content(const String &p_class_name, const String &p_description);

	TaskHelpTooltip();
};

#endif // LIMBOAI_GDEXTENSION

#endif // TASK_HELP_TOOLTIP_H

#endif // TOOLS_ENABLED