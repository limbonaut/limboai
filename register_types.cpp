/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"

#include "blackboard.h"
#include "bt/actions/bt_action.h"
#include "bt/actions/bt_console_print.h"
#include "bt/actions/bt_fail.h"
#include "bt/actions/bt_random_wait.h"
#include "bt/actions/bt_subtree.h"
#include "bt/actions/bt_wait.h"
#include "bt/actions/bt_wait_ticks.h"
#include "bt/behavior_tree.h"
#include "bt/bt_player.h"
#include "bt/bt_task.h"
#include "bt/composites/bt_composite.h"
#include "bt/composites/bt_dynamic_selector.h"
#include "bt/composites/bt_dynamic_sequence.h"
#include "bt/composites/bt_parallel.h"
#include "bt/composites/bt_random_selector.h"
#include "bt/composites/bt_random_sequence.h"
#include "bt/composites/bt_selector.h"
#include "bt/composites/bt_sequence.h"
#include "bt/conditions/bt_condition.h"
#include "bt/decorators/bt_always_fail.h"
#include "bt/decorators/bt_always_succeed.h"
#include "bt/decorators/bt_cooldown.h"
#include "bt/decorators/bt_decorator.h"
#include "bt/decorators/bt_delay.h"
#include "bt/decorators/bt_invert.h"
#include "bt/decorators/bt_probability.h"
#include "bt/decorators/bt_repeat.h"
#include "bt/decorators/bt_repeat_until_failure.h"
#include "bt/decorators/bt_repeat_until_success.h"
#include "bt/decorators/bt_run_limit.h"
#include "bt/decorators/bt_time_limit.h"
#include "limbo_string_names.h"
#include "limbo_utility.h"

#ifdef TOOLS_ENABLED
#include "editor/limbo_ai_editor_plugin.h"
#endif

void register_limboai_types() {
	ClassDB::register_class<Blackboard>();
	ClassDB::register_class<BTTask>();
	ClassDB::register_class<BehaviorTree>();
	ClassDB::register_class<BTPlayer>();

	ClassDB::register_class<BTComposite>();
	ClassDB::register_class<BTSequence>();
	ClassDB::register_class<BTDynamicSequence>();
	ClassDB::register_class<BTDynamicSelector>();
	ClassDB::register_class<BTSelector>();
	ClassDB::register_class<BTRandomSelector>();
	ClassDB::register_class<BTRandomSequence>();
	ClassDB::register_class<BTSelector>();
	ClassDB::register_class<BTParallel>();

	ClassDB::register_class<BTDecorator>();
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

	ClassDB::register_class<BTAction>();
	ClassDB::register_class<BTFail>();
	ClassDB::register_class<BTWait>();
	ClassDB::register_class<BTRandomWait>();
	ClassDB::register_class<BTWaitTicks>();
	ClassDB::register_class<BTSubtree>();
	ClassDB::register_class<BTConsolePrint>();

	ClassDB::register_class<BTCondition>();

#ifdef TOOLS_ENABLED
	EditorPlugins::add_by_type<LimboAIEditorPlugin>();
#endif

	LimboStringNames::create();
}

void unregister_limboai_types() {
	LimboStringNames::free();
}
