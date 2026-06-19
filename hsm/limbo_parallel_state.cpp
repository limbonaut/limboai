/**
 * limbo_parallel_state.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "limbo_parallel_state.h"

VARIANT_ENUM_CAST(LimboParallelState::UpdateMode);

void LimboParallelState::set_active(bool p_active) {
	ERR_FAIL_COND_MSG(agent == nullptr, "LimboParallelState is not initialized.");

	if (active == p_active) {
		return;
	}

	active = p_active;

	if (active) {
		_enter();
	} else {
		_exit();
	}
}

void LimboParallelState::_enter() {
	set_process(update_mode == UpdateMode::IDLE);
	set_physics_process(update_mode == UpdateMode::PHYSICS);

	LimboState::_enter();

	// Enter child states here
	for (int i = 0; i < get_child_count(); i++) {
		LimboState *c = Object::cast_to<LimboState>(get_child(i));
		if (unlikely(c == nullptr)) {
			ERR_PRINT(vformat("LimboParallelState: Child at index %d is not a LimboState.", i));
		} else {
			c->_enter();
		}
	}
}

void LimboParallelState::_exit() {
	set_process(false);
	set_physics_process(false);

	// Exit child states here
	for (int i = 0; i < get_child_count(); i++) {
		LimboState *c = Object::cast_to<LimboState>(get_child(i));
		if (unlikely(c == nullptr)) {
			ERR_PRINT(vformat("LimboParallelState: Child at index %d is not a LimboState.", i));
		} else {
			c->_exit();
		}
	}

	LimboState::_exit();
}

void LimboParallelState::_update(double p_delta) {
	if (active) {
		LimboState::_update(p_delta);

		// Update child states here
		for (int i = 0; i < get_child_count(); i++) {
			LimboState *c = Object::cast_to<LimboState>(get_child(i));
			if (unlikely(c == nullptr)) {
				ERR_PRINT(vformat("LimboParallelState: Child at index %d is not a LimboState.", i));
			} else {
				c->_update(p_delta);
			}
		}
	}
}

void LimboParallelState::update(double p_delta) {
	updating = true;
	_update(p_delta);
	updating = false;
}

LimboState *LimboParallelState::get_leaf_state() const {
	return const_cast<LimboParallelState *>(this);
}

bool LimboParallelState::_dispatch(const StringName &p_event, const Variant &p_cargo) {
	ERR_FAIL_COND_V(p_event == StringName(), false);

	bool event_consumed = false;

	// Dispatch event to children
	for (int i = 0; i < get_child_count(); i++) {
		LimboState *c = Object::cast_to<LimboState>(get_child(i));
		if (unlikely(c == nullptr)) {
			ERR_PRINT(vformat("LimboParallelState: Child at index %d is not a LimboState.", i));
		} else {
			event_consumed = c->_dispatch(p_event, p_cargo) || event_consumed;
		}
	}

	if (!event_consumed) {
		event_consumed = LimboState::_dispatch(p_event, p_cargo);
	}

	return event_consumed;
}

void LimboParallelState::initialize(Node *p_agent, const Ref<Blackboard> &p_parent_scope) {
	ERR_FAIL_COND(p_agent == nullptr);
	ERR_FAIL_COND_MSG(!is_root(), "LimboParallelState: initialize() must be called on the root HSM.");

	_initialize(p_agent, p_parent_scope);
}

void LimboParallelState::_initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard) {
	ERR_FAIL_COND(p_agent == nullptr);
	ERR_FAIL_COND_MSG(agent != nullptr, "LimboAI: ParallelState already initialized.");

	LimboState::_initialize(p_agent, p_blackboard);

	for (int i = 0; i < get_child_count(); i++) {
		LimboState *c = Object::cast_to<LimboState>(get_child(i));
		if (unlikely(c == nullptr)) {
			ERR_PRINT(vformat("LimboParallelState: Child at index %d is not a LimboState.", i));
		} else {
			c->_initialize(agent, blackboard);
		}
	}
}

void LimboParallelState::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == LW_NAME(update_mode) && !is_root()) {
		// Hide update_mode for non-root.
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void LimboParallelState::_exit_if_not_inside_tree() {
	if (is_active() && !is_inside_tree()) {
		_exit();
	}
}

void LimboParallelState::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_POST_ENTER_TREE: {
			if (was_active && is_root()) {
				// Re-activate the root HSM if it was previously active.
				// Typically, this happens when the node is re-entered scene repeatedly (such as with object pooling).
				set_active(true);
			}
		} break;
		case NOTIFICATION_READY: {
			set_process(active && update_mode == UpdateMode::IDLE);
			set_physics_process(active && update_mode == UpdateMode::PHYSICS);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (is_root()) {
				// Exit the state machine if the root HSM is no longer in the scene tree (except when being reparented).
				// This ensures that resources and signal connections are released if active.
				was_active = is_active();
				if (is_active()) {
					// Check if the HSM node is being deleted.
					bool is_being_deleted = false;
					Node *node = this;
					while (node) {
						if (node->is_queued_for_deletion()) {
							is_being_deleted = true;
							break;
						}
						node = node->get_parent();
					}

					if (is_being_deleted) {
						// Exit the state machine immediately if the HSM is being deleted.
						_exit();
					} else {
						// Use deferred mode to prevent exiting during Node re-parenting.
						// This allows the HSM to remain active when it (or one of its parents) is reparented.
						callable_mp(this, &LimboParallelState::_exit_if_not_inside_tree).call_deferred();
					}
				}
			}
		} break;
		case NOTIFICATION_PROCESS: {
			update(get_process_delta_time());
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			update(get_physics_process_delta_time());
		} break;
	}
}

void LimboParallelState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_update_mode", "mode"), &LimboParallelState::set_update_mode);
	ClassDB::bind_method(D_METHOD("get_update_mode"), &LimboParallelState::get_update_mode);

	ClassDB::bind_method(D_METHOD("get_leaf_state"), &LimboParallelState::get_leaf_state);
	ClassDB::bind_method(D_METHOD("set_active", "active"), &LimboParallelState::set_active);
	ClassDB::bind_method(D_METHOD("update", "delta"), &LimboParallelState::update);
	ClassDB::bind_method(D_METHOD("initialize", "agent", "parent_scope"), &LimboParallelState::initialize, Variant());

	BIND_ENUM_CONSTANT(IDLE);
	BIND_ENUM_CONSTANT(PHYSICS);
	BIND_ENUM_CONSTANT(MANUAL);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "update_mode", PROPERTY_HINT_ENUM, "Idle, Physics, Manual"), "set_update_mode", "get_update_mode");
}

LimboParallelState::LimboParallelState() {
	update_mode = UpdateMode::PHYSICS;
}
