/**
 * blackboard_plan.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "blackboard_plan.h"

#include "../compat/editor_settings.h"
#include "../compat/scene_tree.h"
#include "../compat/translation.h"
#include "../util/limbo_utility.h"

#ifdef LIMBOAI_MODULE
#include "scene/main/node.h"
#ifdef TOOLS_ENABLED
#include "editor/inspector/editor_inspector.h"
#endif // TOOLS_ENABLED
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/editor_inspector.hpp>
#endif // LIMBOAI_GDEXTENSION

bool BlackboardPlan::_set(const StringName &p_name, const Variant &p_value) {
	String name_str = p_name;

#ifdef TOOLS_ENABLED
	// * Editor
	if (var_map.has(p_name)) {
		BBVariable &var = var_map[p_name];
		var.set_value(p_value);
		if (base.is_valid() && p_value == base->get_var(p_name).get_value()) {
			// When user pressed reset property button in inspector...
			var.reset_value_changed();
		}
		return true;
	}
#endif // TOOLS_ENABLED

	// * Mapping
	if (name_str.begins_with("mapping/")) {
		StringName mapped_var_name = name_str.get_slicec('/', 1);
		StringName value = p_value;
		bool prop_list_changed = false;
		if (value == StringName()) {
			if (parent_scope_mapping.has(mapped_var_name)) {
				prop_list_changed = true;
				parent_scope_mapping.erase(mapped_var_name);
			}
		} else {
			if (!parent_scope_mapping.has(mapped_var_name)) {
				prop_list_changed = true;
			}
			parent_scope_mapping[mapped_var_name] = value;
		}
		if (prop_list_changed) {
			notify_property_list_changed();
		}
		return true;
	}

	// * Binding
	if (name_str.begins_with("binding/")) {
		StringName bound_var = name_str.get_slicec('/', 1);
		NodePath value = p_value;
		bool prop_list_changed = false;
		if (value.is_empty()) {
			if (property_bindings.has(bound_var)) {
				prop_list_changed = true;
				property_bindings.erase(bound_var);
			}
		} else {
			if (!property_bindings.has(bound_var)) {
				prop_list_changed = true;
			}
			property_bindings[bound_var] = value;
		}
		if (prop_list_changed) {
			notify_property_list_changed();
		}
	}

	// * Storage
	if (name_str.begins_with("var/")) {
		StringName var_name = name_str.get_slicec('/', 1);
		String what = name_str.get_slicec('/', 2);
		if (!var_map.has(var_name) && what == "name") {
			add_var(var_name, BBVariable());
		}
		if (what == "name") {
			// We don't store variable name with the variable.
		} else if (what == "type") {
			var_map[var_name].set_type((Variant::Type)(int)p_value);
		} else if (what == "value") {
			var_map[var_name].set_value(p_value);
		} else if (what == "hint") {
			var_map[var_name].set_hint((PropertyHint)(int)p_value);
		} else if (what == "hint_string") {
			var_map[var_name].set_hint_string(p_value);
		} else if (what == "property_binding") {
			property_bindings[var_name] = NodePath(p_value);
		} else {
			return false;
		}
		return true;
	}

	return false;
}

bool BlackboardPlan::_get(const StringName &p_name, Variant &r_ret) const {
	String name_str = p_name;

#ifdef TOOLS_ENABLED
	// * Editor
	if (var_map.has(p_name)) {
		if (has_mapping(p_name)) {
			r_ret = "Mapped to " + LimboUtility::get_singleton()->decorate_var(parent_scope_mapping[p_name]);
		} else if (has_property_binding(p_name)) {
			const NodePath &binding = property_bindings[p_name];

			Node *edited_node = Object::cast_to<Node>(EditorInterface::get_singleton()->get_inspector()->get_edited_object());
			if (!edited_node) {
				edited_node = SCENE_TREE()->get_edited_scene_root();
			}
			Node *bound_node = edited_node ? edited_node->get_node_or_null(binding) : nullptr;

			String shortened_path;
			if (bound_node) {
				shortened_path = String(bound_node->get_name()) +
						":" + String(binding.get_concatenated_subnames());
			} else {
				shortened_path = String(binding.get_name(binding.get_name_count() - 1)) +
						":" + String(binding.get_concatenated_subnames());
			}
			r_ret = String::utf8("🔗 ") + shortened_path;
		} else {
			r_ret = var_map[p_name].get_value();
		}
		return true;
	}
#endif // TOOLS_ENABLED

	// * Mapping
	if (name_str.begins_with("mapping/")) {
		StringName mapped_var_name = name_str.get_slicec('/', 1);
		ERR_FAIL_COND_V(mapped_var_name == StringName(), false);
		if (has_mapping(mapped_var_name)) {
			r_ret = parent_scope_mapping[mapped_var_name];
		} else if (has_property_binding(mapped_var_name)) {
			r_ret = RTR("Already bound to property.");
		} else {
			r_ret = StringName();
		}
		return true;
	}

	// * Binding
	if (name_str.begins_with("binding/")) {
		StringName bound_var = name_str.get_slicec('/', 1);
		ERR_FAIL_COND_V(bound_var == StringName(), false);
		if (has_property_binding(bound_var)) {
			r_ret = property_bindings[bound_var];
		} else if (has_mapping(bound_var)) {
			r_ret = RTR("Already mapped to variable.");
		} else {
			r_ret = NodePath();
		}
		return true;
	}

	// * Storage
	if (!name_str.begins_with("var/")) {
		return false;
	}
	StringName var_name = name_str.get_slicec('/', 1);
	String what = name_str.get_slicec('/', 2);
	ERR_FAIL_COND_V(!var_map.has(var_name), false);

	if (what == "name") {
		r_ret = var_name;
	} else if (what == "type") {
		r_ret = var_map[var_name].get_type();
	} else if (what == "value") {
		r_ret = var_map[var_name].get_value();
	} else if (what == "hint") {
		r_ret = var_map[var_name].get_hint();
	} else if (what == "hint_string") {
		r_ret = var_map[var_name].get_hint_string();
	}
	return true;
}

void BlackboardPlan::_get_property_list(List<PropertyInfo> *p_list) const {
	for (const Pair<StringName, BBVariable> &p : var_list) {
		String var_name = p.first;
		BBVariable var = p.second;

#ifdef TOOLS_ENABLED
		// * Editor
		if (!_is_var_nil(var) && !_is_var_private(var_name, var)) {
			if (has_mapping(var_name) || has_property_binding(var_name)) {
				p_list->push_back(PropertyInfo(Variant::STRING, var_name, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
			} else {
				p_list->push_back(PropertyInfo(var.get_type(), var_name, var.get_hint(), var.get_hint_string(), PROPERTY_USAGE_EDITOR));
			}
		}
#endif // TOOLS_ENABLED

		// * Storage
		if (is_derived() && (!var.is_value_changed() || var.get_value() == base->var_map[var_name].get_value())) {
			// Don't store variable if it's not modified in a derived plan.
			// Variable is considered modified when it's marked as changed and its value is different from the base plan.
			continue;
		}
		p_list->push_back(PropertyInfo(Variant::STRING, "var/" + var_name + "/name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
		p_list->push_back(PropertyInfo(Variant::INT, "var/" + var_name + "/type", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
		p_list->push_back(PropertyInfo(var.get_type(), "var/" + var_name + "/value", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
		p_list->push_back(PropertyInfo(Variant::INT, "var/" + var_name + "/hint", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
		p_list->push_back(PropertyInfo(Variant::STRING, "var/" + var_name + "/hint_string", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
	}

	// * Mapping
	if (is_mapping_enabled()) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Mapping", PROPERTY_HINT_NONE, "mapping/", PROPERTY_USAGE_GROUP));
		for (const Pair<StringName, BBVariable> &p : var_list) {
			if (_is_var_private(p.first, p.second)) {
				continue;
			}
			if (unlikely(has_property_binding(p.first))) {
				p_list->push_back(PropertyInfo(Variant::STRING, "mapping/" + p.first, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
			} else {
				// Serialize only non-empty mappings.
				PropertyUsageFlags usage = has_mapping(p.first) ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_EDITOR;
				p_list->push_back(PropertyInfo(Variant::STRING_NAME, "mapping/" + p.first, PROPERTY_HINT_NONE, "", usage));
			}
		}
	}

	// * Binding
	p_list->push_back(PropertyInfo(Variant::NIL, "Binding", PROPERTY_HINT_NONE, "binding/", PROPERTY_USAGE_GROUP));
	for (const Pair<StringName, BBVariable> &p : var_list) {
		if (_is_var_nil(p.second) || _is_var_private(p.first, p.second)) {
			continue;
		}
		if (unlikely(has_mapping(p.first))) {
			p_list->push_back(PropertyInfo(Variant::STRING, "binding/" + p.first, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
		} else {
			PropertyUsageFlags usage = has_property_binding(p.first) ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_EDITOR;
			// PROPERTY_HINT_LINK is used to signal that NodePath should point to a property.
			// Our inspector plugin will know how to handle it.
			p_list->push_back(PropertyInfo(Variant::NODE_PATH, "binding/" + p.first, PROPERTY_HINT_LINK, itos(p.second.get_type()), usage));
		}
	}
}

bool BlackboardPlan::_property_can_revert(const StringName &p_name) const {
	if (String(p_name).begins_with("mapping/")) {
		return true;
	}
	return base.is_valid() && base->var_map.has(p_name);
}

bool BlackboardPlan::_property_get_revert(const StringName &p_name, Variant &r_property) const {
	if (String(p_name).begins_with("mapping/")) {
		r_property = StringName();
		return true;
	}
	if (base->var_map.has(p_name)) {
		r_property = base->var_map[p_name].get_value();
		return true;
	}
	return false;
}

void BlackboardPlan::set_base_plan(const Ref<BlackboardPlan> &p_base) {
	if (p_base == this) {
		WARN_PRINT_ED("BlackboardPlan: Using same resource for derived blackboard plan is not supported.");
		base.unref();
	} else {
		base = p_base;
	}
	sync_with_base_plan();
	notify_property_list_changed();
}

void BlackboardPlan::set_parent_scope_plan_provider(const Callable &p_provider_callable) {
	parent_scope_plan_providers.push(p_provider_callable);
	notify_property_list_changed();
}

bool BlackboardPlan::is_mapping_enabled() const {
	const Callable &provider = parent_scope_plan_providers.get_most_recent_valid();
	Ref<BlackboardPlan> parent_scope = provider.call();
	return parent_scope.is_valid();
}

bool BlackboardPlan::has_mapping(const StringName &p_name) const {
	return is_mapping_enabled() && parent_scope_mapping.has(p_name) && parent_scope_mapping[p_name] != StringName();
}

void BlackboardPlan::set_property_binding(const StringName &p_name, const NodePath &p_path) {
	property_bindings[p_name] = p_path;
	emit_changed();
}

void BlackboardPlan::set_prefetch_nodepath_vars(bool p_enable) {
	prefetch_nodepath_vars = p_enable;
	emit_changed();
}

bool BlackboardPlan::is_prefetching_nodepath_vars() const {
	if (is_derived()) {
		return base->is_prefetching_nodepath_vars();
	} else {
		return prefetch_nodepath_vars;
	}
}

void BlackboardPlan::add_var(const StringName &p_name, const BBVariable &p_var) {
	ERR_FAIL_COND(p_name == StringName());
	ERR_FAIL_COND(var_map.has(p_name));
	var_map.insert(p_name, p_var);
	var_list.push_back(Pair<StringName, BBVariable>(p_name, p_var));
	notify_property_list_changed();
	emit_changed();
}

void BlackboardPlan::remove_var(const StringName &p_name) {
	ERR_FAIL_COND(!var_map.has(p_name));
	var_list.erase(Pair<StringName, BBVariable>(p_name, var_map[p_name]));
	var_map.erase(p_name);
	notify_property_list_changed();
	emit_changed();
}

BBVariable BlackboardPlan::get_var(const StringName &p_name) {
	ERR_FAIL_COND_V(!var_map.has(p_name), BBVariable());
	return var_map.get(p_name);
}

Pair<StringName, BBVariable> BlackboardPlan::get_var_by_index(int p_index) {
	Pair<StringName, BBVariable> ret;
	ERR_FAIL_INDEX_V(p_index, (int)var_map.size(), ret);
	return var_list.get(p_index);
}

TypedArray<StringName> BlackboardPlan::list_vars() const {
	TypedArray<StringName> ret;
	for (const Pair<StringName, BBVariable> &p : var_list) {
		ret.append(p.first);
	}
	return ret;
}

StringName BlackboardPlan::get_var_name(const BBVariable &p_var) const {
	for (const Pair<StringName, BBVariable> &p : var_list) {
		if (p.second == p_var) {
			return p.first;
		}
	}
	return StringName();
}

bool BlackboardPlan::is_valid_var_name(const StringName &p_name) const {
	String name_str = p_name;
	if (name_str.begins_with("resource_")) {
		return false;
	}
	return name_str.is_valid_identifier() && !var_map.has(p_name);
}

void BlackboardPlan::rename_var(const StringName &p_name, const StringName &p_new_name) {
	if (p_name == p_new_name) {
		return;
	}

	ERR_FAIL_COND(!is_valid_var_name(p_new_name));
	ERR_FAIL_COND(!var_map.has(p_name));
	ERR_FAIL_COND(var_map.has(p_new_name));

	BBVariable var = var_map[p_name];
	Pair<StringName, BBVariable> new_entry(p_new_name, var);
	Pair<StringName, BBVariable> old_entry(p_name, var);
	var_list.find(old_entry)->set(new_entry);

	var_map.erase(p_name);
	var_map.insert(p_new_name, var);

	if (parent_scope_mapping.has(p_name)) {
		parent_scope_mapping[p_new_name] = parent_scope_mapping[p_name];
		parent_scope_mapping.erase(p_name);
	}

	notify_property_list_changed();
	emit_changed();
}

void BlackboardPlan::move_var(int p_index, int p_new_index) {
	ERR_FAIL_INDEX(p_index, (int)var_map.size());
	ERR_FAIL_INDEX(p_new_index, (int)var_map.size());

	if (p_index == p_new_index) {
		return;
	}

	List<Pair<StringName, BBVariable>>::Element *E = var_list.front();
	for (int i = 0; i < p_index; i++) {
		E = E->next();
	}
	List<Pair<StringName, BBVariable>>::Element *E2 = var_list.front();
	for (int i = 0; i < p_new_index; i++) {
		E2 = E2->next();
	}

	var_list.move_before(E, E2);
	if (p_new_index > p_index) {
		var_list.move_before(E2, E);
	}

	notify_property_list_changed();
	emit_changed();
}

void BlackboardPlan::sync_with_base_plan() {
	if (base.is_null()) {
		return;
	}

	bool changed = false;

	// Sync variables with the base plan.
	for (const Pair<StringName, BBVariable> &p : base->var_list) {
		const StringName &base_name = p.first;
		const BBVariable &base_var = p.second;

		if (!var_map.has(base_name)) {
			add_var(base_name, base_var.duplicate());
			changed = true;
			continue;
		}

		BBVariable var = var_map[base_name];
		if (!var.is_same_prop_info(base_var)) {
			var.copy_prop_info(base_var);
			changed = true;
		}
		if ((!var.is_value_changed() && var.get_value() != base_var.get_value()) ||
				(var.get_value().get_type() != base_var.get_type())) {
			// Reset value according to base plan.
			var.set_value(base_var.get_value());
			var.reset_value_changed();
			changed = true;
		}
	}

	// Erase variables that do not exist in the base plan.
	List<StringName> erase_list;
	for (const Pair<StringName, BBVariable> &p : var_list) {
		if (!base->has_var(p.first)) {
			erase_list.push_back(p.first);
			changed = true;
		}
	}
	while (erase_list.size()) {
		remove_var(erase_list.front()->get());
		erase_list.pop_front();
	}

	// Sync order of variables.
	// Glossary: E - element of current plan, B - element of base plan, F - element of current plan (used for forward search).
	ERR_FAIL_COND(base->var_list.size() != var_list.size());
	List<Pair<StringName, BBVariable>>::Element *B = base->var_list.front();
	for (List<Pair<StringName, BBVariable>>::Element *E = var_list.front(); E; E = E->next()) {
		if (E->get().first != B->get().first) {
			List<Pair<StringName, BBVariable>>::Element *F = E->next();
			while (F) {
				if (F->get().first == B->get().first) {
					var_list.move_before(F, E);
					E = F;
					break;
				}
				F = F->next();
			}
		}
		B = B->next();
	}

	if (changed) {
		notify_property_list_changed();
		emit_changed();
	}
}

Ref<Blackboard> BlackboardPlan::create_blackboard(Node *p_prefetch_root, const Ref<Blackboard> &p_parent_scope, Node *p_prefetch_root_for_base_plan) {
	ERR_FAIL_COND_V(p_prefetch_root == nullptr && prefetch_nodepath_vars, memnew(Blackboard));
	Ref<Blackboard> bb = memnew(Blackboard);
	bb->set_parent(p_parent_scope);
	populate_blackboard(bb, true, p_prefetch_root, p_prefetch_root_for_base_plan);
	return bb;
}

void BlackboardPlan::populate_blackboard(const Ref<Blackboard> &p_blackboard, bool overwrite, Node *p_prefetch_root, Node *p_prefetch_root_for_base_plan) {
	ERR_FAIL_COND(p_prefetch_root == nullptr && prefetch_nodepath_vars);
	ERR_FAIL_COND(p_blackboard.is_null());
	for (const Pair<StringName, BBVariable> &p : var_list) {
		if (p_blackboard->has_local_var(p.first) && !overwrite) {
#ifdef DEBUG_ENABLED
			Variant::Type existing_type = p_blackboard->get_var(p.first).get_type();
			Variant::Type planned_type = p.second.get_type();
			if (existing_type != planned_type && existing_type != Variant::NIL && planned_type != Variant::NIL && !(existing_type == Variant::OBJECT && planned_type == Variant::NODE_PATH)) {
				WARN_PRINT(vformat("BlackboardPlan: Not overwriting %s as it already exists in the blackboard, but it has a different type than planned (%s vs %s). File: %s",
						LimboUtility::get_singleton()->decorate_var(p.first), Variant::get_type_name(existing_type), Variant::get_type_name(planned_type), get_path()));
			}
#endif
			continue;
		}
		bool is_bound = has_property_binding(p.first) || (is_derived() && get_base_plan()->has_property_binding(p.first));
		bool has_mapping = parent_scope_mapping.has(p.first);
		bool do_prefetch = !is_bound && !has_mapping && prefetch_nodepath_vars;

		// Add a variable duplicate to the blackboard, optionally with NodePath prefetch.
		BBVariable var = p.second.duplicate(true);
		if (unlikely(do_prefetch && p.second.get_type() == Variant::NODE_PATH)) {
			Node *prefetch_root = !p_prefetch_root_for_base_plan || !is_derived() || is_derived_var_changed(p.first) ? p_prefetch_root : p_prefetch_root_for_base_plan;
			Node *n = prefetch_root->get_node_or_null(p.second.get_value());
			if (n != nullptr) {
				var.set_value(n);
			} else {
				ERR_PRINT(vformat("BlackboardPlan: Prefetch failed for variable $%s with value: %s", p.first, p.second.get_value()));
				var.set_value(Variant());
			}
		}
		p_blackboard->assign_var(p.first, var);

		if (has_mapping) {
			StringName target_var = parent_scope_mapping[p.first];
			if (target_var != StringName()) {
				ERR_CONTINUE_MSG(p_blackboard->get_parent().is_null(), vformat("BlackboardPlan: Cannot link variable %s to parent scope because the parent scope is not set.", LimboUtility::get_singleton()->decorate_var(p.first)));
				p_blackboard->link_var(p.first, p_blackboard->get_parent(), target_var);
			}
		} else if (is_bound) {
			// Bind variable to a property of a scene node.
			NodePath binding_path;
			Node *binding_root;
			if (has_property_binding(p.first)) {
				binding_path = property_bindings[p.first];
				binding_root = p_prefetch_root;
			} else {
				binding_path = get_base_plan()->property_bindings[p.first];
				binding_root = p_prefetch_root_for_base_plan;
			}
			ERR_CONTINUE_MSG(binding_path.get_subname_count() != 1, vformat("BlackboardPlan: Can't bind variable %s using property path that contains multiple sub-names: %s", LimboUtility::get_singleton()->decorate_var(p.first), binding_path));
			NodePath node_path{ binding_path.get_concatenated_names() };
			StringName prop_name = binding_path.get_subname(0);
			// TODO: Implement binding for base plan as well.
			Node *n = binding_root->get_node_or_null(node_path);
			ERR_CONTINUE_MSG(n == nullptr, vformat("BlackboardPlan: Binding failed for variable %s using property path: %s", LimboUtility::get_singleton()->decorate_var(p.first), binding_path));
			var.bind(n, prop_name);
		}
	}
}

void BlackboardPlan::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_prefetch_nodepath_vars", "enable"), &BlackboardPlan::set_prefetch_nodepath_vars);
	ClassDB::bind_method(D_METHOD("is_prefetching_nodepath_vars"), &BlackboardPlan::is_prefetching_nodepath_vars);

	ClassDB::bind_method(D_METHOD("set_base_plan", "blackboard_plan"), &BlackboardPlan::set_base_plan);
	ClassDB::bind_method(D_METHOD("get_base_plan"), &BlackboardPlan::get_base_plan);
	ClassDB::bind_method(D_METHOD("is_derived"), &BlackboardPlan::is_derived);
	ClassDB::bind_method(D_METHOD("sync_with_base_plan"), &BlackboardPlan::sync_with_base_plan);
	ClassDB::bind_method(D_METHOD("set_parent_scope_plan_provider", "callable"), &BlackboardPlan::set_parent_scope_plan_provider);
	ClassDB::bind_method(D_METHOD("get_parent_scope_plan_provider"), &BlackboardPlan::get_parent_scope_plan_provider);
	ClassDB::bind_method(D_METHOD("create_blackboard", "prefetch_root", "parent_scope", "prefetch_root_for_base_plan"), &BlackboardPlan::create_blackboard, DEFVAL(Ref<Blackboard>()), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("populate_blackboard", "blackboard", "overwrite", "prefetch_root", "prefetch_root_for_base_plan"), &BlackboardPlan::populate_blackboard, DEFVAL(Variant()));

	// To avoid cluttering the member namespace, we do not export unnecessary properties in this class.
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "prefetch_nodepath_vars", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_prefetch_nodepath_vars", "is_prefetching_nodepath_vars");
}

BlackboardPlan::BlackboardPlan() {
}
