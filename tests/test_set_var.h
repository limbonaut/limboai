/**
 * test_set_var.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_SET_VAR_H
#define TEST_SET_VAR_H

#include "core/variant/variant.h"
#include "limbo_test.h"

#include "modules/limboai/blackboard/bb_param/bb_param.h"
#include "modules/limboai/blackboard/bb_param/bb_variant.h"
#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/bt/tasks/blackboard/bt_set_var.h"
#include "modules/limboai/bt/tasks/bt_task.h"
#include "tests/test_macros.h"

namespace TestSetVar {

TEST_CASE("[Modules][LimboAI] BTSetVar") {
	Ref<BTSetVar> sv = memnew(BTSetVar);
	Ref<Blackboard> bb = memnew(Blackboard);
	Node *dummy = memnew(Node);

	sv->initialize(dummy, bb);

	SUBCASE("When variable is not set") {
		ERR_PRINT_OFF;
		sv->set_variable("");
		CHECK(sv->execute(0.01666) == BTTask::FAILURE);
		ERR_PRINT_ON;
	}

	SUBCASE("With variable set") {
		Ref<BBVariant> value = memnew(BBVariant);
		sv->set_value(value);
		sv->set_variable("var");

		SUBCASE("When setting to a provided value") {
			value->set_value_source(BBParam::SAVED_VALUE);
			value->set_saved_value(123);
			CHECK(sv->execute(0.01666) == BTTask::SUCCESS);
			CHECK(bb->get_var("var", 0) == Variant(123));
		}
		SUBCASE("When assigning value of another blackboard variable") {
			value->set_value_source(BBParam::BLACKBOARD_VAR);

			SUBCASE("BB variable is empty") {
				ERR_PRINT_OFF;
				value->set_variable("");
				CHECK(sv->execute(0.01666) == BTTask::FAILURE);
				ERR_PRINT_ON;
			}
			SUBCASE("BB variable doesn't exist") {
				ERR_PRINT_OFF;
				Variant initial_value = Variant(777);
				bb->set_var("var", initial_value);
				value->set_variable("not_found");
				CHECK(sv->execute(0.01666) == BTTask::FAILURE);
				CHECK(bb->get_var("var", 0) == initial_value); // * Check initial value is intact.
				ERR_PRINT_ON;
			}
			SUBCASE("BB variable exists") {
				value->set_variable("compare_var");
				bb->set_var("compare_var", 123);
				CHECK(sv->execute(0.01666) == BTTask::SUCCESS);
				CHECK(bb->get_var("var", 0) == Variant(123));
			}
		}
	}
}

} //namespace TestSetVar

#endif // TEST_SET_VAR_H
