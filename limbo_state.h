/* limbo_state.h */

#ifndef LIMBO_STATE_H
#define LIMBO_STATE_H

#include "blackboard.h"
#include "core/object/class_db.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/variant/variant.h"
#include "scene/main/node.h"

class LimboHSM;

class LimboState : public Node {
	GDCLASS(LimboState, Node);

private:
	struct GuardCallback {
		Object *obj = nullptr;
		StringName func;
		Array binds;
	};

	Object *agent;
	Ref<Blackboard> blackboard;
	HashMap<String, StringName> handlers;
	GuardCallback guard;

protected:
	friend LimboHSM;
	bool active;

	static void _bind_methods();

	void _notification(int p_what);

	void _set_blackboard_data(Dictionary p_value) { blackboard->set_data(p_value.duplicate()); }
	Dictionary _get_blackboard_data() const { return blackboard->get_data(); }

	virtual void _initialize(Object *p_agent, const Ref<Blackboard> &p_blackboard);

	virtual void _setup();
	virtual void _enter();
	virtual void _exit();
	virtual void _update(float p_delta);

	GDVIRTUAL0(_setup);
	GDVIRTUAL0(_enter);
	GDVIRTUAL0(_exit);
	GDVIRTUAL1(_update, float);

	void add_event_handler(const String &p_event, const StringName &p_method);

public:
	static const String EVENT_FINISHED;

	Ref<Blackboard> get_blackboard() const { return blackboard; }

	Object *get_agent() const { return agent; }
	void set_agent(Object *p_agent) { agent = p_agent; }

	virtual bool dispatch(const String &p_event, const Variant &p_cargo);

	LimboState *named(String p_name);
	// LimboState *call_on_setup(Object *p_object, const StringName &p_method) {}
	LimboState *call_on_enter(Object *p_object, const StringName &p_method);
	LimboState *call_on_exit(Object *p_object, const StringName &p_method);
	LimboState *call_on_update(Object *p_object, const StringName &p_method);

	String event_finished() const { return EVENT_FINISHED; }
	LimboState *get_root() const;
	bool is_active() const { return active; }

	void set_guard_func(Object *p_object, const StringName &p_func, const Array &p_binds = Array());
	void clear_guard_func();

	LimboState();
};

#endif // LIMBO_STATE_H