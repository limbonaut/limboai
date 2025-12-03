/**
 * goap_world_state.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef GOAP_WORLD_STATE_H
#define GOAP_WORLD_STATE_H

#include "../blackboard/blackboard.h"

#ifdef LIMBOAI_MODULE
#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>
using namespace godot;
#endif // LIMBOAI_GDEXTENSION

class GOAPWorldState : public RefCounted {
	GDCLASS(GOAPWorldState, RefCounted);

private:
	Dictionary state;

	static bool _values_match(const Variant &p_a, const Variant &p_b);

protected:
	static void _bind_methods();

public:
	void set_fact(const StringName &p_name, const Variant &p_value);
	Variant get_fact(const StringName &p_name, const Variant &p_default = Variant()) const;
	bool has_fact(const StringName &p_name) const;
	void erase_fact(const StringName &p_name);
	void clear();

	void set_state(const Dictionary &p_state) { state = p_state; }
	Dictionary get_state() const { return state; }

	TypedArray<StringName> get_fact_names() const;

	// Populate from Blackboard
	void populate_from_blackboard(const Ref<Blackboard> &p_blackboard, const TypedArray<StringName> &p_fact_names);

	// GOAP-specific methods
	bool satisfies(const Ref<GOAPWorldState> &p_goal) const;
	int distance_to(const Ref<GOAPWorldState> &p_goal) const;
	Ref<GOAPWorldState> duplicate() const;

	// Apply effects (returns new state, doesn't modify this)
	Ref<GOAPWorldState> apply_effects(const Dictionary &p_effects) const;

	// Compute a hash for this world state (for use in hash maps)
	uint32_t compute_hash() const;

	// Equality check for hash map comparison
	bool equals(const Ref<GOAPWorldState> &p_other) const;

	GOAPWorldState() {}
};

#endif // GOAP_WORLD_STATE_H
