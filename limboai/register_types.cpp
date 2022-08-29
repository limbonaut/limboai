/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"

#include "bt/bt_action.h"
#include "bt/bt_always_fail.h"
#include "bt/bt_always_succeed.h"
#include "bt/bt_composite.h"
#include "bt/bt_condition.h"
#include "bt/bt_cooldown.h"
#include "bt/bt_decorator.h"
#include "bt/bt_delay.h"
#include "bt/bt_dynamic_selector.h"
#include "bt/bt_dynamic_sequence.h"
#include "bt/bt_fail.h"
#include "bt/bt_invert.h"
#include "bt/bt_parallel.h"
#include "bt/bt_probability.h"
#include "bt/bt_random_selector.h"
#include "bt/bt_random_sequence.h"
#include "bt/bt_repeat.h"
#include "bt/bt_repeat_until_failure.h"
#include "bt/bt_repeat_until_success.h"
#include "bt/bt_run_limit.h"
#include "bt/bt_selector.h"
#include "bt/bt_sequence.h"
#include "bt/bt_task.h"
#include "bt/bt_time_limit.h"
#include "limbo_string_names.h"
#include "limbo_utility.h"

void register_limboai_types() {
	ClassDB::register_class<BTTask>();

	ClassDB::register_class<BTComposite>();
	ClassDB::register_class<BTDecorator>();
	ClassDB::register_class<BTAction>();
	ClassDB::register_class<BTCondition>();

	ClassDB::register_class<BTSequence>();
	ClassDB::register_class<BTDynamicSequence>();
	ClassDB::register_class<BTDynamicSelector>();
	ClassDB::register_class<BTSelector>();
	ClassDB::register_class<BTRandomSelector>();
	ClassDB::register_class<BTRandomSequence>();
	ClassDB::register_class<BTSelector>();
	ClassDB::register_class<BTParallel>();

	ClassDB::register_class<BTInvert>();
	ClassDB::register_class<BTAlwaysFail>();
	ClassDB::register_class<BTAlwaysSucceed>();
	ClassDB::register_class<BTDelay>();
	ClassDB::register_class<BTRepeat>();
	ClassDB::register_class<BTRepeatUntilFailure>();
	ClassDB::register_class<BTRepeatUntilSuccess>();
	ClassDB::register_class<BTRunLimit>();
	ClassDB::register_class<BTTimeLimit>();
	ClassDB::register_class<BTCooldown>();
	ClassDB::register_class<BTProbability>();

	ClassDB::register_class<BTFail>();

	LimboStringNames::create();
}

void unregister_limboai_types() {
	LimboStringNames::free();
}
