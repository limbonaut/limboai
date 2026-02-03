/**
 * task_help_tooltip.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#ifdef LIMBOAI_GDEXTENSION

#include "task_help_tooltip.h"

#include "../compat/editor_scale.h"
#include "../util/limbo_doc_data.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/text_server.hpp>

void TaskHelpTooltip::_bind_methods() {
}

void TaskHelpTooltip::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			Ref<StyleBox> sb = get_theme_stylebox("panel", "TooltipPanel");
			if (sb.is_valid()) {
				add_theme_stylebox_override("panel", sb);
			}
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			if (rich_text) {
				Ref<Font> doc_font = get_theme_font("doc", "EditorFonts");
				if (doc_font.is_valid()) {
					rich_text->add_theme_font_override("normal_font", doc_font);
				}

				Ref<Font> doc_bold_font = get_theme_font("doc_bold", "EditorFonts");
				if (doc_bold_font.is_valid()) {
					rich_text->add_theme_font_override("bold_font", doc_bold_font);
				}

				Ref<Font> doc_italic_font = get_theme_font("doc_italic", "EditorFonts");
				if (doc_italic_font.is_valid()) {
					rich_text->add_theme_font_override("italics_font", doc_italic_font);
				}

				int font_size = get_theme_font_size("doc_size", "EditorFonts");
				if (font_size > 0) {
					rich_text->add_theme_font_size_override("normal_font_size", font_size);
					rich_text->add_theme_font_size_override("bold_font_size", font_size);
					rich_text->add_theme_font_size_override("italics_font_size", font_size);
				}

				Color font_color = get_theme_color("font_color", "TooltipLabel");
				rich_text->add_theme_color_override("default_color", font_color);
			}
		} break;
	}
}

TaskHelpTooltip *TaskHelpTooltip::make_tooltip(const String &p_class_or_script_path, const String &p_description) {
	String desc = p_description;

	// If no description provided, try to fetch it from LimboDocData.
	if (desc.is_empty()) {
		desc = LimboDocData::get_class_description(p_class_or_script_path);
	}

	if (desc.is_empty()) {
		// No documentation available.
		return nullptr;
	}

	TaskHelpTooltip *tooltip = memnew(TaskHelpTooltip);

	// Determine display name.
	String display_name;
	if (p_class_or_script_path.begins_with("res://")) {
		display_name = p_class_or_script_path.get_file().get_basename();
	} else {
		display_name = p_class_or_script_path;
	}

	tooltip->set_content(display_name, desc);
	return tooltip;
}

void TaskHelpTooltip::set_content(const String &p_class_name, const String &p_description) {
	class_name = p_class_name;

	String bbcode;

	// Add class name as header.
	if (!p_class_name.is_empty()) {
		bbcode += "[b]" + p_class_name + "[/b]\n";
	}

	// Add description.
	if (!p_description.is_empty()) {
		bbcode += p_description;
	}

	rich_text->set_text(bbcode);

	// Set a reasonable max width for the tooltip.
	real_t max_width = 400.0 * EDSCALE;
	rich_text->set_custom_minimum_size(Size2(max_width, 0));
}

TaskHelpTooltip::TaskHelpTooltip() {
	set_mouse_filter(MOUSE_FILTER_PASS);

	rich_text = memnew(RichTextLabel);
	rich_text->set_fit_content(true);
	rich_text->set_mouse_filter(MOUSE_FILTER_PASS);
	rich_text->set_use_bbcode(true);
	rich_text->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	rich_text->set_selection_enabled(false);
	add_child(rich_text);
}

#endif // LIMBOAI_GDEXTENSION

#endif // TOOLS_ENABLED