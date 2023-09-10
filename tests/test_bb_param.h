/**
 * test_bb_param.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_BB_PARAM_H
#define TEST_BB_PARAM_H

#include "core/string/node_path.h"
#include "limbo_test.h"

#include "modules/limboai/blackboard/bb_param/bb_node.h"
#include "modules/limboai/blackboard/bb_param/bb_param.h"
#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/bt/tasks/bt_task.h"
#include "tests/test_macros.h"

namespace TestBBParam {

TEST_CASE("[Modules][LimboAI] BBParam") {
	Ref<BBParam> param = memnew(BBParam);
	Node *dummy = memnew(Node);
	Ref<Blackboard> bb = memnew(Blackboard);

	SUBCASE("Test with a value and common data types") {
		param->set_value_source(BBParam::SAVED_VALUE);

		param->set_saved_value(123);
		CHECK(param->get_value(dummy, bb) == Variant(123));

		param->set_saved_value("test");
		CHECK(param->get_value(dummy, bb) == Variant("test"));

		param->set_saved_value(3.14);
		CHECK(param->get_value(dummy, bb) == Variant(3.14));
	}
	SUBCASE("Test with a BB variable") {
		param->set_value_source(BBParam::BLACKBOARD_VAR);
		param->set_variable("test_var");

		SUBCASE("With integer") {
			bb->set_var("test_var", 123);
			CHECK(param->get_value(dummy, bb) == Variant(123));
		}
		SUBCASE("With String") {
			bb->set_var("test_var", "test");
			CHECK(param->get_value(dummy, bb) == Variant("test"));
		}
		SUBCASE("With float") {
			bb->set_var("test_var", 3.14);
			CHECK(param->get_value(dummy, bb) == Variant(3.14));
		}
		SUBCASE("When variable doesn't exist") {
			ERR_PRINT_OFF;
			CHECK(param->get_value(dummy, bb, "default_value") == Variant("default_value"));
			ERR_PRINT_ON;
		}
	}

	memdelete(dummy);
}

} //namespace TestBBParam

#endif // TEST_BB_PARAM_H
