/**
 * blackboard.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "bb_variable.h"

#ifdef LIMBOAI_MODULE
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/typed_array.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

class Blackboard : public RefCounted {
	GDCLASS(Blackboard, RefCounted);

private:
	HashMap<StringName, BBVariable> data;
	Ref<Blackboard> parent;

protected:
	static void _bind_methods();

#ifdef LIMBOAI_GDEXTENSION
	String _to_string() const { return "<" + get_class() + "#" + itos(get_instance_id()) + ">"; }
#endif

public:
	void set_parent(const Ref<Blackboard> &p_blackboard) { parent = p_blackboard; }
	Ref<Blackboard> get_parent() const { return parent; }

	Ref<Blackboard> top() const;

	Variant get_var(const StringName &p_name, const Variant &p_default = Variant(), bool p_complain = true) const;
	void set_var(const StringName &p_name, const Variant &p_value);
	bool has_var(const StringName &p_name) const;
	_FORCE_INLINE_ bool has_local_var(const StringName &p_name) const { return data.has(p_name); }
	void erase_var(const StringName &p_name);
	void clear() { data.clear(); }
	TypedArray<StringName> list_vars() const;
	void print_state() const;

	Dictionary get_vars_as_dict() const;
	void populate_from_dict(const Dictionary &p_dictionary);

	void bind_var_to_property(const StringName &p_name, Object *p_object, const StringName &p_property, bool p_create = false);
	void unbind_var(const StringName &p_name);

	void assign_var(const StringName &p_name, const BBVariable &p_var);

	void link_var(const StringName &p_name, const Ref<Blackboard> &p_target_blackboard, const StringName &p_target_var, bool p_create = false);
};

#endif // BLACKBOARD_H
