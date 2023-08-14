/**
 * bt_pause_animation.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_pause_animation.h"

//**** Setters / Getters

void BTPauseAnimation::set_animation_player(Ref<BBNode> p_animation_player) {
	animation_player_param = p_animation_player;
	emit_changed();
	if (Engine::get_singleton()->is_editor_hint() && animation_player_param.is_valid()) {
		animation_player_param->connect(SNAME("changed"), Callable(this, SNAME("emit_changed")));
	}
}

//**** Task Implementation

String BTPauseAnimation::get_configuration_warning() const {
	String warning = BTAction::get_configuration_warning();
	if (!warning.is_empty()) {
		warning += "\n";
	}

	if (animation_player_param.is_null()) {
		warning += "Animation Player parameter is not set.\n";
	} else {
		if (animation_player_param->get_value_source() == BBParam::SAVED_VALUE && animation_player_param->get_saved_value().is_zero()) {
			warning += "Path to AnimationPlayer node is not set.\n";
		} else if (animation_player_param->get_value_source() == BBParam::BLACKBOARD_VAR && animation_player_param->get_variable().is_empty()) {
			warning += "AnimationPlayer blackboard variable is not set.\n";
		}
	}

	return warning;
}

String BTPauseAnimation::_generate_name() const {
	return "PauseAnimation";
}

void BTPauseAnimation::_setup() {
	setup_failed = true;
	ERR_FAIL_COND_MSG(animation_player_param.is_null(), "BTPauseAnimation: AnimationPlayer parameter is not set.");
	animation_player = Object::cast_to<AnimationPlayer>(animation_player_param->get_value(get_agent(), get_blackboard()));
	ERR_FAIL_COND_MSG(animation_player == nullptr, "BTPauseAnimation: Failed to get AnimationPlayer.");
	setup_failed = false;
}

int BTPauseAnimation::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(setup_failed == true, FAILURE, "BTPauseAnimation: _setup() failed - returning FAILURE.");
	animation_player->pause();
	return SUCCESS;
}

//**** Godot

void BTPauseAnimation::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_animation_player", "p_anim_player"), &BTPauseAnimation::set_animation_player);
	ClassDB::bind_method(D_METHOD("get_animation_player"), &BTPauseAnimation::get_animation_player);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "animation_player", PROPERTY_HINT_RESOURCE_TYPE, "BBNode"), "set_animation_player", "get_animation_player");
}
