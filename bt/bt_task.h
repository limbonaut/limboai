/* bt_task.h */

#ifndef BTTASK_H
#define BTTASK_H

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "modules/limboai/blackboard.h"
#include "scene/resources/texture.h"

class BTTask : public Resource {
	GDCLASS(BTTask, Resource);

public:
	enum {
		FRESH,
		RUNNING,
		FAILURE,
		SUCCESS,
	};

private:
	friend class BehaviorTree;

	String custom_name;
	Node *agent;
	Ref<Blackboard> blackboard;
	BTTask *parent;
	Vector<Ref<BTTask>> children;
	int status;
	double elapsed;

	Array _get_children() const;
	void _set_children(Array children);

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _setup() {}
	virtual void _enter() {}
	virtual void _exit() {}
	virtual int _tick(double p_delta) { return FAILURE; }

	GDVIRTUAL0RC(String, _generate_name);
	GDVIRTUAL0(_setup);
	GDVIRTUAL0(_enter);
	GDVIRTUAL0(_exit);
	GDVIRTUAL1R(int, _tick, double);
	GDVIRTUAL0RC(String, _get_configuration_warning);

public:
	virtual bool editor_can_reload_from_file() override { return false; }

	Node *get_agent() const { return agent; }
	void set_agent(Node *p_agent) { agent = p_agent; }

	String get_custom_name() const { return custom_name; }
	void set_custom_name(const String &p_name);
	String get_task_name() const;

	Ref<Blackboard> get_blackboard() const { return blackboard; }
	Ref<BTTask> get_parent() const { return Ref<BTTask>(parent); }
	Ref<BTTask> get_root() const;
	bool is_root() const { return parent == nullptr; }

	virtual Ref<BTTask> clone() const;
	virtual void initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard);
	virtual String get_configuration_warning() const;

	int execute(double p_delta);
	void cancel();
	int get_status() const { return status; }
	double get_elapsed_time() const { return elapsed; };

	Ref<BTTask> get_child(int p_idx) const;
	int get_child_count() const;
	void add_child(Ref<BTTask> p_child);
	void add_child_at_index(Ref<BTTask> p_child, int p_idx);
	void remove_child(Ref<BTTask> p_child);
	void remove_child_at_index(int p_idx);
	bool has_child(const Ref<BTTask> &p_child) const;
	bool is_descendant_of(const Ref<BTTask> &p_task) const;
	int get_child_index(const Ref<BTTask> &p_child) const;
	Ref<BTTask> next_sibling() const;

	void print_tree(int p_initial_tabs = 0) const;

	BTTask();
	~BTTask();
};

#endif // BTTASK_H