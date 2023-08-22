/**
 * action_banner.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "action_banner.h"

#include "scene/gui/button.h"

void ActionBanner::set_text(const String &p_text) {
	message->set_text(p_text);
}

String ActionBanner::get_text() const {
	return message->get_text();
}

void ActionBanner::close() {
	queue_free();
}

void ActionBanner::add_action(const String &p_name, const Callable &p_action, bool p_auto_close) {
	Button *action_btn = memnew(Button);
	action_btn->set_text(p_name);
	action_btn->connect(SNAME("pressed"), callable_mp(this, &ActionBanner::_execute_action).bind(p_action, p_auto_close));
	hbox->add_child(action_btn);
}

void ActionBanner::_execute_action(const Callable &p_action, bool p_auto_close) {
	Callable::CallError ce;
	Variant ret;
	p_action.callp(nullptr, 0, ret, ce);

	if (p_auto_close) {
		queue_free();
	}
}

void ActionBanner::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED: {
			icon->set_texture(get_theme_icon(SNAME("StatusWarning"), SNAME("EditorIcons")));
		} break;
	}
}

void ActionBanner::_bind_methods() {
}

ActionBanner::ActionBanner() {
	add_theme_constant_override("margin_bottom", 4);
	add_theme_constant_override("margin_top", 4);
	add_theme_constant_override("margin_left", 10);
	add_theme_constant_override("margin_right", 10);

	hbox = memnew(HBoxContainer);
	hbox->add_theme_constant_override("hseparation", 8);
	add_child(hbox);

	icon = memnew(TextureRect);
	icon->set_expand_mode(TextureRect::ExpandMode::EXPAND_KEEP_SIZE);
	icon->set_stretch_mode(TextureRect::StretchMode::STRETCH_KEEP_CENTERED);
	hbox->add_child(icon);

	message = memnew(Label);
	message->set_text(vformat(TTR("User task folder doesn't exist")));
	message->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	hbox->add_child(message);

	Control *spacer = memnew(Control);
	spacer->set_custom_minimum_size(Size2(0, 16));
	hbox->add_child(spacer);
}
