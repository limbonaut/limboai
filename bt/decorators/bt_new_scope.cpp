/* bt_new_scope.cpp */

#include "bt_new_scope.h"
#include "core/error/error_macros.h"
#include "core/os/memory.h"
#include "core/string/ustring.h"
#include "modules/limboai/blackboard.h"

void BTNewScope::initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard) {
	ERR_FAIL_COND(p_agent == nullptr);
	ERR_FAIL_COND(p_blackboard == nullptr);

	Ref<Blackboard> bb = memnew(Blackboard);

	bb->set_data(blackboard_data.duplicate());
	bb->set_parent_scope(p_blackboard);

	BTDecorator::initialize(p_agent, bb);
}

int BTNewScope::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	return get_child(0)->execute(p_delta);
}

void BTNewScope::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_set_blackboard_data", "p_data"), &BTNewScope::_set_blackboard_data);
	ClassDB::bind_method(D_METHOD("_get_blackboard_data"), &BTNewScope::_get_blackboard_data);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "_blackboard_data"), "_set_blackboard_data", "_get_blackboard_data");
}