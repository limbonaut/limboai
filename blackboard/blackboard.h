/* blackboard.h */

#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "scene/main/node.h"

class Blackboard : public RefCounted {
	GDCLASS(Blackboard, RefCounted);

private:
	Dictionary data;
	Ref<Blackboard> parent;

protected:
	static void _bind_methods();

public:
	void set_data(const Dictionary &p_value) { data = p_value; }
	Dictionary get_data() const { return data; }

	void set_parent_scope(const Ref<Blackboard> &p_blackboard) { parent = p_blackboard; }
	Ref<Blackboard> get_parent_scope() const { return parent; }

	Ref<Blackboard> top() const;

	Variant get_var(const Variant &p_key, const Variant &p_default) const;
	void set_var(const Variant &p_key, const Variant &p_value);
	bool has_var(const Variant &p_key) const;
	void erase_var(const Variant &p_key);

	void prefetch_nodepath_vars(Node *p_node);
};

#endif // BLACKBOARD_H