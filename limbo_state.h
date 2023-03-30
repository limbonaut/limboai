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
#include "core/variant/callable.h"
#include "core/variant/variant.h"
#include "scene/main/node.h"

class LimboHSM;

class LimboState : public Node {
	GDCLASS(LimboState, Node);

private:
	Node *agent;
	Ref<Blackboard> blackboard;
	HashMap<String, Callable> handlers;
	Callable guard_callable;

protected:
	friend LimboHSM;

	bool active;

	static void _bind_methods();

	void _notification(int p_what);

	void _set_blackboard_data(Dictionary p_value) { blackboard->set_data(p_value.duplicate()); }
	Dictionary _get_blackboard_data() const { return blackboard->get_data(); }

	virtual void _initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard);

	virtual void _setup();
	virtual void _enter();
	virtual void _exit();
	virtual void _update(float p_delta);

	GDVIRTUAL0(_setup);
	GDVIRTUAL0(_enter);
	GDVIRTUAL0(_exit);
	GDVIRTUAL1(_update, float);

	void add_event_handler(const String &p_event, const Callable &p_handler);

public:
	static const String EVENT_FINISHED;

	Ref<Blackboard> get_blackboard() const { return blackboard; }

	Node *get_agent() const { return agent; }
	void set_agent(Node *p_agent) { agent = p_agent; }

	virtual bool dispatch(const String &p_event, const Variant &p_cargo);

	LimboState *named(String p_name);
	LimboState *call_on_enter(const Callable &p_callable);
	LimboState *call_on_exit(const Callable &p_callable);
	LimboState *call_on_update(const Callable &p_callable);

	_FORCE_INLINE_ String event_finished() const { return EVENT_FINISHED; }
	LimboState *get_root() const;
	bool is_active() const { return active; }

	void set_guard(const Callable &p_guard_callable);
	void clear_guard();

	LimboState();
};

#endif // LIMBO_STATE_H