/**
 * bt_play_animation.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_PLAY_ANIMATION_H
#define BT_PLAY_ANIMATION_H

#include "bt_action.h"

#include "modules/limboai/blackboard/bb_param/bb_node.h"

#include "core/object/object.h"
#include "scene/animation/animation_player.h"

class BTPlayAnimation : public BTAction {
	GDCLASS(BTPlayAnimation, BTAction);

private:
	Ref<BBNode> animation_player_param;
	StringName animation_name;
	double await_completion = 0.0;
	double blend = -1.0;
	double speed = 1.0;
	bool from_end = false;

	AnimationPlayer *animation_player = nullptr;
	bool setup_failed = false;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual void _setup() override;
	virtual void _enter() override;
	virtual int _tick(double p_delta) override;

public:
	void set_animation_player(Ref<BBNode> p_animation_player);
	Ref<BBNode> get_animation_player() const { return animation_player_param; }

	void set_animation_name(StringName p_animation_name);
	StringName get_animation_name() const { return animation_name; }

	void set_await_completion(double p_await_completion);
	double get_await_completion() const { return await_completion; }

	void set_blend(double p_blend);
	double get_blend() const { return blend; }

	void set_speed(double p_speed);
	double get_speed() const { return speed; }

	void set_from_end(bool p_from_end);
	bool get_from_end() const { return from_end; }

	virtual String get_configuration_warning() const override;
};

#endif // BT_PLAY_ANIMATION