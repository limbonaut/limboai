/**
 * test_set_agent_property.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_SET_AGENT_PROPERTY_H
#define TEST_SET_AGENT_PROPERTY_H

#include "limbo_test.h"

#include "modules/limboai/blackboard/bb_param/bb_param.h"
#include "modules/limboai/blackboard/bb_param/bb_variant.h"
#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/bt/tasks/bt_task.h"
#include "modules/limboai/bt/tasks/scene/bt_set_agent_property.h"

#include "core/os/memory.h"

namespace TestSetAgentProperty {

TEST_CASE("[Modules][LimboAI] BTSetAgentProperty") {
	Ref<BTSetAgentProperty> sap = memnew(BTSetAgentProperty);
	Node *agent = memnew(Node);
	Ref<Blackboard> bb = memnew(Blackboard);
	sap->initialize(agent, bb);

	sap->set_property("process_priority"); // * property that will be set by the task
	Ref<BBVariant> value_param = memnew(BBVariant);
	value_param->set_value_source(BBParam::SAVED_VALUE);
	value_param->set_saved_value(7);
	sap->set_value(value_param);

	SUBCASE("With integer") {
		CHECK(sap->execute(0.01666) == BTTask::SUCCESS);
		CHECK(agent->get_process_priority() == 7);
	}
	SUBCASE("When value is not set") {
		sap->set_value(nullptr);
		ERR_PRINT_OFF;
		CHECK(sap->execute(0.01666) == BTTask::FAILURE);
		ERR_PRINT_ON;
	}
	SUBCASE("When property is empty") {
		sap->set_property("");
		ERR_PRINT_OFF;
		CHECK(sap->execute(0.01666) == BTTask::FAILURE);
		ERR_PRINT_ON;
	}
	SUBCASE("When property doesn't exist") {
		sap->set_property("not_found");
		ERR_PRINT_OFF;
		CHECK(sap->execute(0.01666) == BTTask::FAILURE);
		ERR_PRINT_ON;
	}
	SUBCASE("With StringName and String") {
		value_param->set_saved_value("TestName");
		sap->set_property("name");
		CHECK(sap->execute(0.01666) == BTTask::SUCCESS);
		CHECK(agent->get_name() == "TestName");
	}
	SUBCASE("With blackboard variable") {
		value_param->set_value_source(BBParam::BLACKBOARD_VAR);
		value_param->set_variable("priority");

		SUBCASE("With proper BB variable") {
			bb->set_var("priority", 8);
			CHECK(sap->execute(0.01666) == BTTask::SUCCESS);
			CHECK(agent->get_process_priority() == 8);
		}
		SUBCASE("With BB variable of wrong type") {
			bb->set_var("priority", "high");
			ERR_PRINT_OFF;
			CHECK(sap->execute(0.01666) == BTTask::FAILURE);
			ERR_PRINT_ON;
			CHECK(agent->get_process_priority() == 0);
		}
		SUBCASE("When BB variable doesn't exist") {
			value_param->set_variable("not_found");
			ERR_PRINT_OFF;
			CHECK(sap->execute(0.01666) == BTTask::FAILURE);
			ERR_PRINT_ON;
			CHECK(agent->get_process_priority() == 0);
		}
		SUBCASE("When BB variable isn't set") {
			value_param->set_variable("");
			ERR_PRINT_OFF;
			CHECK(sap->execute(0.01666) == BTTask::FAILURE);
			ERR_PRINT_ON;
			CHECK(agent->get_process_priority() == 0);
		}
	}

	memdelete(agent);
}

} //namespace TestSetAgentProperty

#endif // TEST_SET_AGENT_PROPERTY_H
