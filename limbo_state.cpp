/* limbo_state.cpp */

#include "limbo_state.h"
#include "core/array.h"
#include "core/class_db.h"
#include "core/error_macros.h"
#include "core/object.h"
#include "core/typedefs.h"
#include "core/variant.h"
#include "limbo_string_names.h"

const String LimboState::EVENT_FINISHED = "finished";

LimboState *LimboState::get_root() const {
	const LimboState *state = this;
	while (get_parent() && get_parent()->is_class("LimboState")) {
		state = Object::cast_to<LimboState>(get_parent());
	}
	return const_cast<LimboState *>(state);
}

LimboState *LimboState::named(String p_name) {
	set_name(p_name);
	return this;
};

void LimboState::_setup() {
	if (get_script_instance() &&
			get_script_instance()->has_method(LimboStringNames::get_singleton()->_setup)) {
		get_script_instance()->call(LimboStringNames::get_singleton()->_setup);
	}
	emit_signal(LimboStringNames::get_singleton()->setup);
};

void LimboState::_enter() {
	active = true;
	if (get_script_instance() &&
			get_script_instance()->has_method(LimboStringNames::get_singleton()->_enter)) {
		get_script_instance()->call(LimboStringNames::get_singleton()->_enter);
	}
	emit_signal(LimboStringNames::get_singleton()->entered);
};

void LimboState::_exit() {
	if (!active) {
		return;
	}
	if (get_script_instance() &&
			get_script_instance()->has_method(LimboStringNames::get_singleton()->_exit)) {
		get_script_instance()->call(LimboStringNames::get_singleton()->_exit);
	}
	emit_signal(LimboStringNames::get_singleton()->exited);
	active = false;
};

void LimboState::_update(float p_delta) {
	if (get_script_instance() &&
			get_script_instance()->has_method(LimboStringNames::get_singleton()->_update)) {
		get_script_instance()->call(LimboStringNames::get_singleton()->_update, p_delta);
	}
	emit_signal(LimboStringNames::get_singleton()->updated, p_delta);
};

void LimboState::initialize(Object *p_agent, const Ref<Blackboard> &p_blackboard) {
	ERR_FAIL_COND(p_agent == nullptr);
	ERR_FAIL_COND(!p_blackboard.is_valid());
	agent = p_agent;
	blackboard = p_blackboard;
	_setup();
};

bool LimboState::dispatch(const String &p_event, const Variant &p_cargo) {
	ERR_FAIL_COND_V(p_event.empty(), false);
	bool result = false;
	if (handlers.size() > 0 && handlers.has(p_event)) {
		if (p_cargo.get_type() == Variant::NIL) {
			result = call(handlers[p_event]);
		} else {
			result = call(handlers[p_event], p_cargo);
		}
	}
	return result;
}

void LimboState::add_event_handler(const String &p_event, const StringName &p_method) {
	ERR_FAIL_COND(p_event.empty());
	ERR_FAIL_COND(!has_method(p_method));
	handlers.insert(p_event, p_method);
}

LimboState *LimboState::call_on_enter(Object *p_object, const StringName &p_method) {
	ERR_FAIL_COND_V(p_object == nullptr, this);
	ERR_FAIL_COND_V(!p_object->has_method(p_method), this);
	connect(LimboStringNames::get_singleton()->entered, p_object, p_method);
	return this;
}

LimboState *LimboState::call_on_exit(Object *p_object, const StringName &p_method) {
	ERR_FAIL_COND_V(p_object == nullptr, this);
	ERR_FAIL_COND_V(!p_object->has_method(p_method), this);
	connect(LimboStringNames::get_singleton()->exited, p_object, p_method);
	return this;
}

LimboState *LimboState::call_on_update(Object *p_object, const StringName &p_method) {
	ERR_FAIL_COND_V(p_object == nullptr, this);
	ERR_FAIL_COND_V(!p_object->has_method(p_method), this);
	connect(LimboStringNames::get_singleton()->updated, p_object, p_method);
	return this;
}

void LimboState::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_EXIT_TREE: {
			if (active) {
				_exit();
			}
		} break;
	}
}

void LimboState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_root"), &LimboState::get_root);
	ClassDB::bind_method(D_METHOD("event_finished"), &LimboState::event_finished);
	ClassDB::bind_method(D_METHOD("is_active"), &LimboState::is_active);
	ClassDB::bind_method(D_METHOD("_setup"), &LimboState::_setup);
	ClassDB::bind_method(D_METHOD("_enter"), &LimboState::_enter);
	ClassDB::bind_method(D_METHOD("_exit"), &LimboState::_exit);
	ClassDB::bind_method(D_METHOD("_update", "p_delta"), &LimboState::_update);
	ClassDB::bind_method(D_METHOD("initialize", "p_agent", "p_blackboard"), &LimboState::initialize);
	ClassDB::bind_method(D_METHOD("dispatch", "p_event", "p_cargo"), &LimboState::dispatch, Variant());
	ClassDB::bind_method(D_METHOD("named", "p_name"), &LimboState::named);
	ClassDB::bind_method(D_METHOD("add_event_handler", "p_event", "p_method"), &LimboState::add_event_handler);
	ClassDB::bind_method(D_METHOD("call_on_enter", "p_object", "p_method"), &LimboState::call_on_enter);
	ClassDB::bind_method(D_METHOD("call_on_exit", "p_object", "p_method"), &LimboState::call_on_exit);
	ClassDB::bind_method(D_METHOD("call_on_update", "p_object", "p_method"), &LimboState::call_on_update);

	BIND_VMETHOD(MethodInfo("_setup"));
	BIND_VMETHOD(MethodInfo("_enter"));
	BIND_VMETHOD(MethodInfo("_exit"));
	BIND_VMETHOD(MethodInfo("_update", PropertyInfo(Variant::REAL, "p_delta")));

	ADD_SIGNAL(MethodInfo("setup"));
	ADD_SIGNAL(MethodInfo("entered"));
	ADD_SIGNAL(MethodInfo("exited"));
	ADD_SIGNAL(MethodInfo("updated", PropertyInfo(Variant::REAL, "p_delta")));
};

LimboState::LimboState() {
	agent = nullptr;
	active = false;

	set_process(false);
	set_physics_process(false);
	set_process_input(false);
}
