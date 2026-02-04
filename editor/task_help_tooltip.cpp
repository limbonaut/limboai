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

#include <godot_cpp/classes/class_db_singleton.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/text_server.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/window.hpp>

bool TaskHelpTooltip::_is_tooltip_visible = false;

void TaskHelpTooltip::_bind_methods() {
}

Control *TaskHelpTooltip::_make_invisible_control() {
	Control *control = memnew(Control);
	control->set_visible(false);
	return control;
}

void TaskHelpTooltip::_start_timer() {
	if (timer && timer->is_inside_tree() && timer->is_stopped()) {
		timer->start();
	}
}

void TaskHelpTooltip::_target_gui_input(const Ref<InputEvent> &p_event) {
	// Scrolling closes the tooltip.
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		switch (mb->get_button_index()) {
			case MouseButton::MOUSE_BUTTON_WHEEL_UP:
			case MouseButton::MOUSE_BUTTON_WHEEL_DOWN:
			case MouseButton::MOUSE_BUTTON_WHEEL_LEFT:
			case MouseButton::MOUSE_BUTTON_WHEEL_RIGHT:
				queue_free();
				break;
			default:
				break;
		}
	}
}

void TaskHelpTooltip::_update_content_height() {
	float content_height = content->get_content_height();
	Ref<StyleBox> style = content->get_theme_stylebox("normal");
	if (style.is_valid()) {
		content_height += style->get_content_margin(SIDE_TOP) + style->get_content_margin(SIDE_BOTTOM);
	}
	content->set_custom_minimum_size(Size2(content->get_custom_minimum_size().x, CLAMP(content_height, content_min_height, content_max_height)));
}

void TaskHelpTooltip::_update_theme() {
	if (title) {
		Ref<Font> doc_bold_font = get_theme_font("doc_bold", "EditorFonts");
		if (doc_bold_font.is_valid()) {
			title->add_theme_font_override("bold_font", doc_bold_font);
		}

		Ref<Font> doc_font = get_theme_font("doc", "EditorFonts");
		if (doc_font.is_valid()) {
			title->add_theme_font_override("normal_font", doc_font);
		}

		int font_size = get_theme_font_size("doc_size", "EditorFonts");
		if (font_size > 0) {
			title->add_theme_font_size_override("normal_font_size", font_size);
			title->add_theme_font_size_override("bold_font_size", font_size);
		}

		Color title_color = get_theme_color("title_color", "EditorHelp");
		title->add_theme_color_override("default_color", title_color);
	}

	if (content) {
		Ref<Font> doc_font = get_theme_font("doc", "EditorFonts");
		if (doc_font.is_valid()) {
			content->add_theme_font_override("normal_font", doc_font);
		}

		Ref<Font> doc_bold_font = get_theme_font("doc_bold", "EditorFonts");
		if (doc_bold_font.is_valid()) {
			content->add_theme_font_override("bold_font", doc_bold_font);
		}

		Ref<Font> doc_italic_font = get_theme_font("doc_italic", "EditorFonts");
		if (doc_italic_font.is_valid()) {
			content->add_theme_font_override("italics_font", doc_italic_font);
		}

		int font_size = get_theme_font_size("doc_size", "EditorFonts");
		if (font_size > 0) {
			content->add_theme_font_size_override("normal_font_size", font_size);
			content->add_theme_font_size_override("bold_font_size", font_size);
			content->add_theme_font_size_override("italics_font_size", font_size);
		}

		Color text_color = get_theme_color("text_color", "EditorHelp");
		content->add_theme_color_override("default_color", text_color);
	}
}

void TaskHelpTooltip::_add_text_to_rt(const String &p_bbcode, RichTextLabel *p_rt) {
	// Get theme resources.
	const Ref<Font> doc_font = get_theme_font("doc", "EditorFonts");
	const Ref<Font> doc_bold_font = get_theme_font("doc_bold", "EditorFonts");
	const Ref<Font> doc_italic_font = get_theme_font("doc_italic", "EditorFonts");
	const Ref<Font> doc_code_font = get_theme_font("doc_source", "EditorFonts");
	const Ref<Font> doc_kbd_font = get_theme_font("doc_keyboard", "EditorFonts");

	const int doc_font_size = get_theme_font_size("doc_size", "EditorFonts");
	const int doc_code_font_size = get_theme_font_size("doc_source_size", "EditorFonts");
	const int doc_kbd_font_size = get_theme_font_size("doc_keyboard_size", "EditorFonts");

	// Get theme colors with sensible fallbacks for when EditorHelp theme is not available.
	const Color font_color = get_theme_color("font_color", "EditorFonts");
	const Color accent_color = get_theme_color("accent_color", "Editor");
	const Color error_color = get_theme_color("error_color", "Editor");

	Color type_color = get_theme_color("type_color", "EditorHelp");
	if (type_color == Color()) {
		type_color = Color(0.47, 0.67, 1.0); // Light blue fallback.
	}

	Color code_color = get_theme_color("code_color", "EditorHelp");
	if (code_color == Color()) {
		// Fallback: blend accent color with font color.
		code_color = accent_color.lerp(font_color.is_equal_approx(Color()) ? Color(0.8, 0.8, 0.8) : font_color, 0.6);
	}

	Color kbd_color = get_theme_color("kbd_color", "EditorHelp");
	if (kbd_color == Color()) {
		kbd_color = font_color.is_equal_approx(Color()) ? Color(0.8, 0.8, 0.8) : font_color;
	}

	const Color code_dark_color = Color(code_color, 0.8);

	// For [code] tags, blend code_color with error_color for reddish tint (like Godot does).
	Color code_inline_color = code_color.lerp(error_color.is_equal_approx(Color()) ? Color(1.0, 0.3, 0.3) : error_color, 0.6);

	Color link_color = get_theme_color("link_color", "EditorHelp");
	if (link_color == Color()) {
		link_color = Color(0.3, 0.6, 1.0); // Blue link fallback.
	}

	Color link_method_color = accent_color;
	if (link_method_color == Color()) {
		link_method_color = Color(0.3, 0.7, 0.4); // Green fallback.
	}

	const Color link_property_color = link_color.lerp(link_method_color, 0.25);

	Color code_bg_color = get_theme_color("code_bg_color", "EditorHelp");
	if (code_bg_color == Color()) {
		code_bg_color = Color(0.0, 0.0, 0.0, 0.0); // Transparent fallback - no visible background.
	}

	Color kbd_bg_color = get_theme_color("kbd_bg_color", "EditorHelp");
	if (kbd_bg_color == Color()) {
		kbd_bg_color = Color(0.3, 0.3, 0.3, 0.6);
	}

	Color param_bg_color = get_theme_color("param_bg_color", "EditorHelp");
	if (param_bg_color == Color()) {
		param_bg_color = Color(0.2, 0.3, 0.4, 0.6);
	}

	String bbcode = p_bbcode.strip_edges();

	// Remove carriage returns.
	bbcode = bbcode.replace("\r", "");

	// Handle codeblocks - select the correct code examples (GDScript by default).
	bbcode = bbcode.replace("[gdscript", "[codeblock lang=gdscript");
	bbcode = bbcode.replace("[/gdscript]", "[/codeblock]");

	// Remove C# examples.
	int pos = bbcode.find("[csharp");
	while (pos != -1) {
		int end_pos = bbcode.find("[/csharp]");
		if (end_pos == -1) {
			break;
		}
		bbcode = bbcode.left(pos) + bbcode.substr(end_pos + 9);
		while (pos < bbcode.length() && bbcode[pos] == '\n') {
			bbcode = bbcode.left(pos) + bbcode.substr(pos + 1);
		}
		pos = bbcode.find("[csharp");
	}

	// Remove codeblocks wrapper tags.
	bbcode = bbcode.replace("[codeblocks]\n", "");
	bbcode = bbcode.replace("\n[/codeblocks]", "");
	bbcode = bbcode.replace("[codeblocks]", "");
	bbcode = bbcode.replace("[/codeblocks]", "");

	// Remove `\n` after codeblock end because `\n` is replaced by `\n\n` later.
	bbcode = bbcode.replace("[/codeblock]\n", "[/codeblock]");

	List<String> tag_stack;

	pos = 0;
	while (pos < bbcode.length()) {
		int brk_pos = bbcode.find("[", pos);

		if (brk_pos < 0) {
			brk_pos = bbcode.length();
		}

		if (brk_pos > pos) {
			p_rt->add_text(bbcode.substr(pos, brk_pos - pos).replace("\n", "\n\n"));
		}

		if (brk_pos == bbcode.length()) {
			break; // Nothing else to add.
		}

		int brk_end = bbcode.find("]", brk_pos + 1);

		if (brk_end == -1) {
			p_rt->add_text(bbcode.substr(brk_pos).replace("\n", "\n\n"));
			break;
		}

		const String tag = bbcode.substr(brk_pos + 1, brk_end - brk_pos - 1);

		if (tag.begins_with("/")) {
			bool tag_ok = tag_stack.size() > 0 && tag_stack.front()->get() == tag.substr(1);

			if (!tag_ok) {
				p_rt->add_text("[");
				pos = brk_pos + 1;
				continue;
			}

			tag_stack.pop_front();
			pos = brk_end + 1;
			if (tag == "/img") {
				// Nothing to do.
			} else if (tag == "/url") {
				p_rt->pop(); // meta
				p_rt->pop(); // color
			} else {
				p_rt->pop();
			}
		} else if (tag.begins_with("method ") || tag.begins_with("constructor ") || tag.begins_with("operator ") || tag.begins_with("member ") || tag.begins_with("signal ") || tag.begins_with("enum ") || tag.begins_with("constant ") || tag.begins_with("annotation ") || tag.begins_with("theme_item ")) {
			const int tag_end = tag.find(" ");
			const String link_tag = tag.left(tag_end);
			const String link_target = tag.substr(tag_end + 1).strip_edges();

			Color target_color = link_color;
			if (link_tag == "method" || link_tag == "constructor" || link_tag == "operator") {
				target_color = link_method_color;
			} else if (link_tag == "member" || link_tag == "signal" || link_tag == "theme_item") {
				target_color = link_property_color;
			}

			// Use monospace font for references.
			if (doc_code_font.is_valid()) {
				p_rt->push_font(doc_code_font);
			}
			if (doc_code_font_size > 0) {
				p_rt->push_font_size(doc_code_font_size);
			}
			p_rt->push_color(target_color);

			if (link_tag == "method") {
				p_rt->add_text(link_target + String("()"));
			} else {
				p_rt->add_text(link_target);
			}

			p_rt->pop(); // color
			if (doc_code_font_size > 0) {
				p_rt->pop(); // font_size
			}
			if (doc_code_font.is_valid()) {
				p_rt->pop(); // font
			}

			pos = brk_end + 1;
		} else if (tag.begins_with("param ")) {
			const int tag_end = tag.find(" ");
			const String param_name = tag.substr(tag_end + 1).strip_edges();

			// Use monospace font with background color.
			if (doc_code_font.is_valid()) {
				p_rt->push_font(doc_code_font);
			}
			if (doc_code_font_size > 0) {
				p_rt->push_font_size(doc_code_font_size);
			}
			p_rt->push_bgcolor(param_bg_color);
			p_rt->push_color(code_color);

			p_rt->add_text(param_name);

			p_rt->pop(); // color
			p_rt->pop(); // bgcolor
			if (doc_code_font_size > 0) {
				p_rt->pop(); // font_size
			}
			if (doc_code_font.is_valid()) {
				p_rt->pop(); // font
			}

			pos = brk_end + 1;
		} else if (LimboDocData::get_class_doc(tag) != nullptr || ClassDBSingleton::get_singleton()->class_exists(tag)) {
			// Class reference like [BTTask] or [Node2D].
			if (doc_code_font.is_valid()) {
				p_rt->push_font(doc_code_font);
			}
			if (doc_code_font_size > 0) {
				p_rt->push_font_size(doc_code_font_size);
			}
			p_rt->push_color(type_color);

			p_rt->add_text(tag);

			p_rt->pop(); // color
			if (doc_code_font_size > 0) {
				p_rt->pop(); // font_size
			}
			if (doc_code_font.is_valid()) {
				p_rt->pop(); // font
			}

			pos = brk_end + 1;
		} else if (tag == "b") {
			// Bold font.
			if (doc_bold_font.is_valid()) {
				p_rt->push_font(doc_bold_font);
			} else {
				p_rt->push_bold();
			}

			pos = brk_end + 1;
			tag_stack.push_front(tag);
		} else if (tag == "i") {
			// Italics font.
			if (doc_italic_font.is_valid()) {
				p_rt->push_font(doc_italic_font);
			} else {
				p_rt->push_italics();
			}

			pos = brk_end + 1;
			tag_stack.push_front(tag);
		} else if (tag == "code" || tag.begins_with("code ")) {
			int end_pos = bbcode.find("[/code]", brk_end + 1);
			if (end_pos < 0) {
				end_pos = bbcode.length();
			}

			// Use monospace font with background color (only if not transparent).
			if (doc_code_font.is_valid()) {
				p_rt->push_font(doc_code_font);
			}
			if (doc_code_font_size > 0) {
				p_rt->push_font_size(doc_code_font_size);
			}
			bool has_code_bg = code_bg_color.a > 0.01;
			if (has_code_bg) {
				p_rt->push_bgcolor(code_bg_color);
			}
			p_rt->push_color(code_inline_color);

			p_rt->add_text(bbcode.substr(brk_end + 1, end_pos - (brk_end + 1)));

			p_rt->pop(); // color
			if (has_code_bg) {
				p_rt->pop(); // bgcolor
			}
			if (doc_code_font_size > 0) {
				p_rt->pop(); // font_size
			}
			if (doc_code_font.is_valid()) {
				p_rt->pop(); // font
			}

			pos = end_pos + 7; // len("[/code]")
		} else if (tag == "codeblock" || tag.begins_with("codeblock ")) {
			int end_pos = bbcode.find("[/codeblock]", brk_end + 1);
			if (end_pos < 0) {
				end_pos = bbcode.length();
			}

			const String codeblock_text = bbcode.substr(brk_end + 1, end_pos - (brk_end + 1)).strip_edges();

			// Use monospace font with background color in a table cell.
			if (doc_code_font.is_valid()) {
				p_rt->push_font(doc_code_font);
			}
			if (doc_code_font_size > 0) {
				p_rt->push_font_size(doc_code_font_size);
			}
			p_rt->push_table(1);

			p_rt->push_cell();
			p_rt->set_cell_row_background_color(code_bg_color, Color(code_bg_color, 0.99));
			p_rt->set_cell_padding(Rect2(10 * EDSCALE, 10 * EDSCALE, 10 * EDSCALE, 10 * EDSCALE));
			p_rt->push_color(code_dark_color);

			p_rt->add_text(codeblock_text);

			p_rt->pop(); // color
			p_rt->pop(); // cell

			p_rt->pop(); // table
			if (doc_code_font_size > 0) {
				p_rt->pop(); // font_size
			}
			if (doc_code_font.is_valid()) {
				p_rt->pop(); // font
			}

			pos = end_pos + 12; // len("[/codeblock]")

			// Compensate for `\n` removed before the loop.
			if (pos < bbcode.length()) {
				p_rt->newline();
			}
		} else if (tag == "kbd") {
			int end_pos = bbcode.find("[/kbd]", brk_end + 1);
			if (end_pos < 0) {
				end_pos = bbcode.length();
			}

			// Use keyboard font with custom color and background color.
			if (doc_kbd_font.is_valid()) {
				p_rt->push_font(doc_kbd_font);
			}
			if (doc_kbd_font_size > 0) {
				p_rt->push_font_size(doc_kbd_font_size);
			}
			p_rt->push_bgcolor(kbd_bg_color);
			p_rt->push_color(kbd_color);

			p_rt->add_text(bbcode.substr(brk_end + 1, end_pos - (brk_end + 1)));

			p_rt->pop(); // color
			p_rt->pop(); // bgcolor
			if (doc_kbd_font_size > 0) {
				p_rt->pop(); // font_size
			}
			if (doc_kbd_font.is_valid()) {
				p_rt->pop(); // font
			}

			pos = end_pos + 6; // len("[/kbd]")
		} else if (tag == "center") {
			// Align to center.
			p_rt->push_paragraph(HORIZONTAL_ALIGNMENT_CENTER, Control::TEXT_DIRECTION_AUTO, "");
			pos = brk_end + 1;
			tag_stack.push_front(tag);
		} else if (tag == "br") {
			// Force a line break.
			p_rt->newline();
			pos = brk_end + 1;
		} else if (tag == "u") {
			// Use underline.
			p_rt->push_underline();
			pos = brk_end + 1;
			tag_stack.push_front(tag);
		} else if (tag == "s") {
			// Use strikethrough.
			p_rt->push_strikethrough();
			pos = brk_end + 1;
			tag_stack.push_front(tag);
		} else if (tag == "lb") {
			p_rt->add_text("[");
			pos = brk_end + 1;
		} else if (tag == "rb") {
			p_rt->add_text("]");
			pos = brk_end + 1;
		} else if (tag == "url" || tag.begins_with("url=")) {
			String url;
			if (tag.begins_with("url=")) {
				url = tag.substr(4);
			} else {
				int end = bbcode.find("[", brk_end);
				if (end == -1) {
					end = bbcode.length();
				}
				url = bbcode.substr(brk_end + 1, end - brk_end - 1);
			}

			p_rt->push_color(link_color);
			p_rt->push_meta(url, RichTextLabel::META_UNDERLINE_ON_HOVER);

			pos = brk_end + 1;
			tag_stack.push_front("url");
		} else if (tag.begins_with("img")) {
			int end = bbcode.find("[", brk_end);
			if (end == -1) {
				end = bbcode.length();
			}
			// Skip image content.
			pos = end;
			tag_stack.push_front("img");
		} else if (tag.begins_with("color=")) {
			String col = tag.substr(6);
			Color color = Color::from_string(col, Color());
			p_rt->push_color(color);

			pos = brk_end + 1;
			tag_stack.push_front("color");
		} else if (tag.begins_with("font=")) {
			// Skip custom fonts - use default.
			if (doc_font.is_valid()) {
				p_rt->push_font(doc_font);
			}

			pos = brk_end + 1;
			tag_stack.push_front("font");
		} else {
			p_rt->add_text("["); // Ignore unknown tag.
			pos = brk_pos + 1;
		}
	}

	// Close unclosed tags.
	for (const String &tag : tag_stack) {
		if (tag != "img") {
			p_rt->pop();
		}
	}
}

void TaskHelpTooltip::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_is_tooltip_visible = true;
			_enter_tree_time = Time::get_singleton()->get_ticks_msec();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_is_tooltip_visible = false;
		} break;
		case NOTIFICATION_WM_MOUSE_ENTER: {
			_is_mouse_inside_tooltip = true;
			if (timer) {
				timer->stop();
			}
		} break;
		case NOTIFICATION_WM_MOUSE_EXIT: {
			_is_mouse_inside_tooltip = false;
			_start_timer();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme();
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			// Workaround to hide the tooltip since the window does not receive keyboard events
			// with FLAG_POPUP and FLAG_NO_FOCUS flags.
			if (is_inside_tree()) {
				Input *input = Input::get_singleton();
				if (input->is_action_just_pressed("ui_cancel", true)) {
					queue_free();
					get_viewport()->set_input_as_handled();
				} else if (input->is_anything_pressed()) {
					queue_free();
				} else if (!input->get_last_mouse_velocity().is_zero_approx()) {
					if (!_is_mouse_inside_tooltip && Time::get_singleton()->get_ticks_msec() - _enter_tree_time > 350) {
						_start_timer();
					}
				}
			}
		} break;
	}
}

Control *TaskHelpTooltip::make_tooltip(Control *p_target, const String &p_class_or_script_path, const String &p_description) {
	ERR_FAIL_NULL_V(p_target, _make_invisible_control());

	// Show the custom tooltip only if it is not already visible.
	// The viewport will retrigger make_custom_tooltip() every few seconds
	// because the return control is not visible even if the custom tooltip is displayed.
	if (_is_tooltip_visible || Input::get_singleton()->is_anything_pressed()) {
		return _make_invisible_control();
	}

	String desc = p_description;

	// If no description provided, try to fetch it from LimboDocData.
	if (desc.is_empty()) {
		const LimboDocData::ClassDoc *class_doc = LimboDocData::get_class_doc(p_class_or_script_path);
		if (class_doc) {
			// Prefer full description, fall back to brief.
			if (!class_doc->description.is_empty()) {
				desc = class_doc->description;
			} else {
				desc = class_doc->brief_description;
			}
		}
	}

	if (desc.is_empty()) {
		// No documentation available.
		return _make_invisible_control();
	}

	// Determine display name.
	String display_name;
	if (p_class_or_script_path.begins_with("res://")) {
		display_name = p_class_or_script_path.get_file().get_basename();
	} else {
		display_name = p_class_or_script_path;
	}

	TaskHelpTooltip *tooltip = memnew(TaskHelpTooltip(p_target));
	tooltip->set_content(display_name, desc);
	p_target->add_child(tooltip);

	tooltip->_update_content_height();
	tooltip->popup_under_cursor();

	return _make_invisible_control();
}

void TaskHelpTooltip::set_content(const String &p_class_name, const String &p_description) {
	// Set title.
	if (!p_class_name.is_empty()) {
		title->clear();
		title->push_bold();
		title->add_text(p_class_name);
		title->pop();
		title->show();
	} else {
		title->hide();
	}

	// Set content with BBCode parsing.
	content->clear();
	if (!p_description.is_empty()) {
		_add_text_to_rt(p_description, content);
	}
}

void TaskHelpTooltip::popup_under_cursor() {
	Point2 mouse_pos = get_mouse_position();
	Point2 tooltip_offset = ProjectSettings::get_singleton()->get_setting("display/mouse_cursor/tooltip_position_offset", Point2(10, 10));
	Rect2 r(mouse_pos + tooltip_offset, get_contents_minimum_size());
	r.size = r.size.min(get_max_size());

	// Get usable screen rect for positioning.
	Rect2i vr = DisplayServer::get_singleton()->screen_get_usable_rect();

	if (r.size.x + r.position.x > vr.size.x + vr.position.x) {
		// Place it in the opposite direction. If it fails, just hug the border.
		r.position.x = mouse_pos.x - r.size.x - tooltip_offset.x;

		if (r.position.x < vr.position.x) {
			r.position.x = vr.position.x + vr.size.x - r.size.x;
		}
	} else if (r.position.x < vr.position.x) {
		r.position.x = vr.position.x;
	}

	if (r.size.y + r.position.y > vr.size.y + vr.position.y) {
		// Same as above.
		r.position.y = mouse_pos.y - r.size.y - tooltip_offset.y;

		if (r.position.y < vr.position.y) {
			r.position.y = vr.position.y + vr.size.y - r.size.y;
		}
	} else if (r.position.y < vr.position.y) {
		r.position.y = vr.position.y;
	}

	// When FLAG_POPUP is false, it prevents the editor from losing focus when displaying the tooltip.
	// This way, clicks and double-clicks are still available outside the tooltip.
	set_flag(Window::FLAG_POPUP, false);
	set_flag(Window::FLAG_NO_FOCUS, true);
	popup(r);
}

TaskHelpTooltip::TaskHelpTooltip(Control *p_target) {
	ERR_FAIL_NULL(p_target);

	set_theme_type_variation("TooltipPanel");

	content_min_height = 48 * EDSCALE;
	content_max_height = 360 * EDSCALE;

	vbox = memnew(VBoxContainer);
	vbox->add_theme_constant_override("separation", 0);
	add_child(vbox);

	title = memnew(RichTextLabel);
	title->set_custom_minimum_size(Size2(512 * EDSCALE, 0));
	title->set_fit_content(true);
	title->set_use_bbcode(true);
	title->set_selection_enabled(false);
	title->set_context_menu_enabled(false);
	title->hide();
	vbox->add_child(title);

	content = memnew(RichTextLabel);
	content->set_custom_minimum_size(Size2(512 * EDSCALE, content_min_height));
	content->set_use_bbcode(true);
	content->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	content->set_selection_enabled(false);
	content->set_context_menu_enabled(false);
	content->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	vbox->add_child(content);

	timer = memnew(Timer);
	timer->set_wait_time(0.25);
	timer->connect("timeout", callable_mp(static_cast<Node *>(this), &Node::queue_free));
	add_child(timer);

	p_target->connect("mouse_exited", callable_mp(this, &TaskHelpTooltip::_start_timer));
	p_target->connect("gui_input", callable_mp(this, &TaskHelpTooltip::_target_gui_input));

	set_process_internal(true);
}

TaskHelpTooltip::TaskHelpTooltip() {
	// Default constructor - should not be used directly.
	// Required for GDCLASS registration.
}

#endif // LIMBOAI_GDEXTENSION

#endif // TOOLS_ENABLED