/* limbo_state.h */

#ifndef LIMBO_STATE_H
#define LIMBO_STATE_H

#include "blackboard.h"
#include "core/string_name.h"
#include "core/ustring.h"
#include "core/variant.h"
#include "scene/main/node.h"

// TODO Implement guards

class LimboState : public Node {
	GDCLASS(LimboState, Node);

private:
	Object *agent;
	Ref<Blackboard> blackboard;
	Map<String, StringName> handlers;
	bool active;
	// Guard *guard;

protected:
	static void _bind_methods();

	void _notification(int p_what);

	virtual void _setup();
	virtual void _enter();
	virtual void _exit();
	virtual void _update(float p_delta);

	void add_event_handler(const String &p_event, const StringName &p_method);

public:
	static const String EVENT_FINISHED;

	virtual void initialize(Object *p_agent, const Ref<Blackboard> &p_blackboard);
	virtual bool dispatch(const String &p_event, const Variant &p_cargo);

	LimboState *named(String p_name);
	// LimboState *call_on_setup(Object *p_object, const StringName &p_method) {}
	LimboState *call_on_enter(Object *p_object, const StringName &p_method);
	LimboState *call_on_exit(Object *p_object, const StringName &p_method);
	LimboState *call_on_update(Object *p_object, const StringName &p_method);

	String event_finished() const { return EVENT_FINISHED; }
	LimboState *get_root() const;
	bool is_active() const { return active; }

	LimboState();
};

#endif // LIMBO_STATE_H