/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"

#include "bb_param/bb_aabb.h"
#include "bb_param/bb_array.h"
#include "bb_param/bb_basis.h"
#include "bb_param/bb_bool.h"
#include "bb_param/bb_byte_array.h"
#include "bb_param/bb_color.h"
#include "bb_param/bb_color_array.h"
#include "bb_param/bb_dictionary.h"
#include "bb_param/bb_float.h"
#include "bb_param/bb_int.h"
#include "bb_param/bb_int_array.h"
#include "bb_param/bb_param.h"
#include "bb_param/bb_plane.h"
#include "bb_param/bb_quat.h"
#include "bb_param/bb_real_array.h"
#include "bb_param/bb_rect2.h"
#include "bb_param/bb_string.h"
#include "bb_param/bb_string_array.h"
#include "bb_param/bb_transform.h"
#include "bb_param/bb_transform2d.h"
#include "bb_param/bb_vector2.h"
#include "bb_param/bb_vector2_array.h"
#include "bb_param/bb_vector3.h"
#include "bb_param/bb_vector3_array.h"
#include "blackboard.h"
#include "bt/actions/bt_action.h"
#include "bt/actions/bt_console_print.h"
#include "bt/actions/bt_fail.h"
#include "bt/actions/bt_random_wait.h"
#include "bt/actions/bt_wait.h"
#include "bt/actions/bt_wait_ticks.h"
#include "bt/behavior_tree.h"
#include "bt/bt_player.h"
#include "bt/bt_state.h"
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
#include "bt/decorators/bt_for_each.h"
#include "bt/decorators/bt_invert.h"
#include "bt/decorators/bt_new_scope.h"
#include "bt/decorators/bt_probability.h"
#include "bt/decorators/bt_repeat.h"
#include "bt/decorators/bt_repeat_until_failure.h"
#include "bt/decorators/bt_repeat_until_success.h"
#include "bt/decorators/bt_run_limit.h"
#include "bt/decorators/bt_subtree.h"
#include "bt/decorators/bt_time_limit.h"
#include "core/os/memory.h"
#include "limbo_hsm.h"
#include "limbo_state.h"
#include "limbo_string_names.h"
#include "limbo_utility.h"

#ifdef TOOLS_ENABLED
#include "editor/limbo_ai_editor_plugin.h"
#endif

static LimboUtility *_limbo_utility = nullptr;

void register_limboai_types() {
	ClassDB::register_class<Blackboard>();

	ClassDB::register_class<LimboState>();
	ClassDB::register_class<LimboHSM>();

	ClassDB::register_class<BTTask>();
	ClassDB::register_class<BehaviorTree>();
	ClassDB::register_class<BTPlayer>();
	ClassDB::register_class<BTState>();

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
	ClassDB::register_class<BTForEach>();

	ClassDB::register_class<BTAction>();
	ClassDB::register_class<BTFail>();
	ClassDB::register_class<BTWait>();
	ClassDB::register_class<BTRandomWait>();
	ClassDB::register_class<BTWaitTicks>();
	ClassDB::register_class<BTNewScope>();
	ClassDB::register_class<BTSubtree>();
	ClassDB::register_class<BTConsolePrint>();

	ClassDB::register_class<BTCondition>();

	ClassDB::register_class<BBParam>();
	ClassDB::register_class<BBInt>();
	ClassDB::register_class<BBBool>();
	ClassDB::register_class<BBFloat>();
	ClassDB::register_class<BBString>();
	ClassDB::register_class<BBVector2>();
	ClassDB::register_class<BBRect2>();
	ClassDB::register_class<BBVector3>();
	ClassDB::register_class<BBTransform2D>();
	ClassDB::register_class<BBPlane>();
	ClassDB::register_class<BBQuat>();
	ClassDB::register_class<BBAabb>();
	ClassDB::register_class<BBBasis>();
	ClassDB::register_class<BBTransform>();
	ClassDB::register_class<BBColor>();
	ClassDB::register_class<BBDictionary>();
	ClassDB::register_class<BBArray>();
	ClassDB::register_class<BBByteArray>();
	ClassDB::register_class<BBIntArray>();
	ClassDB::register_class<BBRealArray>();
	ClassDB::register_class<BBColorArray>();
	ClassDB::register_class<BBStringArray>();
	ClassDB::register_class<BBVector2Array>();
	ClassDB::register_class<BBVector3Array>();

	_limbo_utility = memnew(LimboUtility);
	ClassDB::register_class<LimboUtility>();

#ifdef TOOLS_ENABLED
	Engine::get_singleton()->add_singleton(Engine::Singleton("LimboUtility", LimboUtility::get_singleton()));
	EditorPlugins::add_by_type<LimboAIEditorPlugin>();
#endif

	LimboStringNames::create();
}

void unregister_limboai_types() {
	LimboStringNames::free();

	memdelete(_limbo_utility);
}
