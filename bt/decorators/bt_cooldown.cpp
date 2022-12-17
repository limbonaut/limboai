/* bt_cooldown.cpp */

#include "bt_cooldown.h"
#include "core/object/class_db.h"
#include "core/variant/array.h"
#include "scene/main/scene_tree.h"

String BTCooldown::_generate_name() const {
	return vformat("Cooldown %ss", duration);
}

void BTCooldown::_setup() {
	if (cooldown_state_var.is_empty()) {
		cooldown_state_var = vformat("cooldown_%d", rand());
	}
	get_blackboard()->set_var(cooldown_state_var, false);
	if (start_cooled) {
		_chill();
	}
}

int BTCooldown::_tick(float p_delta) {
	ERR_FAIL_COND_V_MSG(get_child_count() == 0, FAILURE, "BT decorator has no child.");
	if (get_blackboard()->get_var(cooldown_state_var, true)) {
		return FAILURE;
	}
	int status = get_child(0)->execute(p_delta);
	if (status == SUCCESS || (trigger_on_failure && status == FAILURE)) {
		_chill();
	}
	return status;
}

void BTCooldown::_chill() {
	get_blackboard()->set_var(cooldown_state_var, true);
	if (timer.is_valid()) {
		timer->set_time_left(duration);
	} else {
		timer = SceneTree::get_singleton()->create_timer(duration, process_pause);
		timer->connect("timeout", callable_mp(this, &BTCooldown::_on_timeout), CONNECT_ONE_SHOT);
	}
}

void BTCooldown::_on_timeout() {
	get_blackboard()->set_var(cooldown_state_var, false);
	timer.unref();
}

void BTCooldown::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_duration", "p_value"), &BTCooldown::set_duration);
	ClassDB::bind_method(D_METHOD("get_duration"), &BTCooldown::get_duration);
	ClassDB::bind_method(D_METHOD("set_process_pause", "p_value"), &BTCooldown::set_process_pause);
	ClassDB::bind_method(D_METHOD("get_process_pause"), &BTCooldown::get_process_pause);
	ClassDB::bind_method(D_METHOD("set_start_cooled", "p_value"), &BTCooldown::set_start_cooled);
	ClassDB::bind_method(D_METHOD("get_start_cooled"), &BTCooldown::get_start_cooled);
	ClassDB::bind_method(D_METHOD("set_trigger_on_failure", "p_value"), &BTCooldown::set_trigger_on_failure);
	ClassDB::bind_method(D_METHOD("get_trigger_on_failure"), &BTCooldown::get_trigger_on_failure);
	ClassDB::bind_method(D_METHOD("set_cooldown_state_var", "p_value"), &BTCooldown::set_cooldown_state_var);
	ClassDB::bind_method(D_METHOD("get_cooldown_state_var"), &BTCooldown::get_cooldown_state_var);
	ClassDB::bind_method(D_METHOD("_on_timeout"), &BTCooldown::_on_timeout);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"), "set_duration", "get_duration");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "process_pause"), "set_process_pause", "get_process_pause");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "start_cooled"), "set_start_cooled", "get_start_cooled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "trigger_on_failure"), "set_trigger_on_failure", "get_trigger_on_failure");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "cooldown_state_var"), "set_cooldown_state_var", "get_cooldown_state_var");
}
