/* bt_task.h */

#ifndef BTTASK_H
#define BTTASK_H

#include "core/array.h"
#include "core/dictionary.h"
#include "core/object.h"
#include "core/reference.h"
#include "core/resource.h"
#include "core/ustring.h"
#include "core/vector.h"
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
	Object *agent;
	Dictionary blackboard;
	BTTask *parent;
	Vector<Ref<BTTask>> children;
	int status;

	Array _get_children() const;
	void _set_children(Array children);

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _setup() {}
	virtual void _enter() {}
	virtual void _exit() {}
	virtual int _tick(float p_delta) { return FAILURE; }

public:
	Object *get_agent() const { return agent; }
	Dictionary get_blackboard() const { return blackboard; }
	Ref<BTTask> get_parent() const { return Ref<BTTask>(parent); }
	bool is_root() const { return parent == nullptr; }
	Ref<BTTask> get_root() const;
	int get_status() const { return status; }
	String get_custom_name() const { return custom_name; }
	void set_custom_name(const String &p_name);
	String get_task_name() const;

	void initialize(Object *p_agent, Dictionary p_blackboard);
	virtual Ref<BTTask> clone() const;
	int execute(float p_delta);
	void cancel();
	Ref<BTTask> get_child(int p_idx) const;
	int get_child_count() const;
	void add_child(Ref<BTTask> p_child);
	void add_child_at_index(Ref<BTTask> p_child, int p_idx);
	void remove_child(Ref<BTTask> p_child);
	bool has_child(const Ref<BTTask> &p_child) const;
	bool is_descendant_of(const Ref<BTTask> &p_task) const;
	int get_child_index(const Ref<BTTask> &p_child) const;
	Ref<BTTask> next_sibling() const;
	virtual String get_configuration_warning() const;
	virtual Ref<Texture> get_icon() const;
	void print_tree(int p_initial_tabs = 0) const;

	BTTask();
	~BTTask();
};

#endif // BTTASK_H