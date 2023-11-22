/**
 * bt_task.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BTTASK_H
#define BTTASK_H

#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/util/limbo_task_db.h"

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/typedefs.h"
#include "core/variant/array.h"
#include "core/variant/binder_common.h"
#include "core/variant/dictionary.h"
#include "scene/resources/texture.h"

/**
 * Base class for BTTask.
 * Note: In order to properly return Status in the _tick virtual method (GDVIRTUAL1R...)
 * we must do VARIANT_ENUM_CAST for Status enum before the actual BTTask class declaration.
 */
class BT : public Resource {
	GDCLASS(BT, Resource);

public:
	enum Status {
		FRESH,
		RUNNING,
		FAILURE,
		SUCCESS,
	};

protected:
	static void _bind_methods();
};

VARIANT_ENUM_CAST(BT::Status)

class BTTask : public BT {
	GDCLASS(BTTask, BT);

private:
	friend class BehaviorTree;

	// Avoid namespace pollution in derived classes.
	struct Data {
		int index = -1;
		String custom_name;
		Node *agent = nullptr;
		Ref<Blackboard> blackboard;
		BTTask *parent = nullptr;
		Vector<Ref<BTTask>> children;
		Status status = FRESH;
		double elapsed = 0.0;
	} data;

	Array _get_children() const;
	void _set_children(Array children);

protected:
	static void _bind_methods();

	virtual String _generate_name() const;
	virtual void _setup() {}
	virtual void _enter() {}
	virtual void _exit() {}
	virtual Status _tick(double p_delta) { return FAILURE; }

	GDVIRTUAL0RC(String, _generate_name);
	GDVIRTUAL0(_setup);
	GDVIRTUAL0(_enter);
	GDVIRTUAL0(_exit);
	GDVIRTUAL1R(Status, _tick, double);
	GDVIRTUAL0RC(PackedStringArray, _get_configuration_warning);

public:
	virtual bool editor_can_reload_from_file() override { return false; }

	Node *get_agent() const { return data.agent; }
	void set_agent(Node *p_agent) { data.agent = p_agent; }

	String get_custom_name() const { return data.custom_name; }
	void set_custom_name(const String &p_name);
	String get_task_name() const;

	Ref<Blackboard> get_blackboard() const { return data.blackboard; }
	Ref<BTTask> get_parent() const { return Ref<BTTask>(data.parent); }
	Ref<BTTask> get_root() const;
	bool is_root() const { return data.parent == nullptr; }

	virtual Ref<BTTask> clone() const;
	virtual void initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard);
	virtual PackedStringArray get_configuration_warnings() const;

	Status execute(double p_delta);
	void abort();
	Status get_status() const { return data.status; }
	double get_elapsed_time() const { return data.elapsed; };

	Ref<BTTask> get_child(int p_idx) const;
	int get_child_count() const;
	int get_child_count_excluding_comments() const;
	void add_child(Ref<BTTask> p_child);
	void add_child_at_index(Ref<BTTask> p_child, int p_idx);
	void remove_child(Ref<BTTask> p_child);
	void remove_child_at_index(int p_idx);
	bool has_child(const Ref<BTTask> &p_child) const;
	bool is_descendant_of(const Ref<BTTask> &p_task) const;
	_FORCE_INLINE_ int get_index() const { return data.index; }
	Ref<BTTask> next_sibling() const;

	void print_tree(int p_initial_tabs = 0) const;

	BTTask();
	~BTTask();
};

#endif // BTTASK_H