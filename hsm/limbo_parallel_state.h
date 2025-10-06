/**
 * limbo_hsm.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBO_PARALLEL_STATE_H
#define LIMBO_PARALLEL_STATE_H

#include "limbo_state.h"

class LimboParallelState : public LimboState {
	GDCLASS(LimboParallelState, LimboState);

public:
	enum UpdateMode : unsigned int {
		IDLE, // automatically call update() during NOTIFICATION_PROCESS
		PHYSICS, // automatically call update() during NOTIFICATION_PHYSICS
		MANUAL, // manually update state machine: user must call update(delta)
	};

private:
	UpdateMode update_mode;
	bool updating = false;
	bool was_active = false;

	void _exit_if_not_inside_tree();

protected:
	static void _bind_methods();

	void _notification(int p_what);
	void _validate_property(PropertyInfo &p_property) const;

	virtual void _initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard) override;
	virtual bool _dispatch(const StringName &p_event, const Variant &p_cargo = Variant()) override;

	virtual void _enter() override;
	virtual void _exit() override;
	virtual void _update(double p_delta) override;

public:
	void set_update_mode(UpdateMode p_mode) { update_mode = p_mode; }
	UpdateMode get_update_mode() const { return update_mode; }

	void set_active(bool p_active);

	LimboState *get_leaf_state() const;

	virtual void initialize(Node *p_agent, const Ref<Blackboard> &p_parent_scope = nullptr);

	void update(double p_delta);

	LimboParallelState();
};

#endif // LIMBO_PARALLEL_STATE_H
