/**
 * bt_pause_animation.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_PAUSE_ANIMATION_H
#define BT_PAUSE_ANIMATION_H

#include "../bt_action.h"

#include "modules/limboai/blackboard/bb_param/bb_node.h"

#include "scene/animation/animation_player.h"

class BTPauseAnimation : public BTAction {
	GDCLASS(BTPauseAnimation, BTAction);
	TASK_CATEGORY(Actions);

private:
	Ref<BBNode> animation_player_param;

	AnimationPlayer *animation_player = nullptr;
	bool setup_failed = false;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual void _setup() override;
	virtual int _tick(double p_delta) override;

public:
	void set_animation_player(Ref<BBNode> p_animation_player);
	Ref<BBNode> get_animation_player() const { return animation_player_param; }

	virtual PackedStringArray get_configuration_warnings() const override;
};

#endif // BT_PAUSE_ANIMATION
