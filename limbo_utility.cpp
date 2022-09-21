/* limbo_utility.cpp */

#include "limbo_utility.h"
#include "core/variant.h"

LimboUtility *LimboUtility::singleton = nullptr;

LimboUtility *LimboUtility::get_singleton() {
	return singleton;
}

String LimboUtility::decorate_var(String p_variable) {
	String var = p_variable.trim_prefix("$").trim_prefix("\"").trim_suffix("\"");
	if (var.find(" ") == -1 and not var.empty()) {
		return vformat("$%s", var);
	} else {
		return vformat("$\"%s\"", var);
	}
}

void LimboUtility::_bind_methods() {
	ClassDB::bind_method(D_METHOD("decorate_var", "p_variable"), &LimboUtility::decorate_var);
}

LimboUtility::LimboUtility() {
	singleton = this;
}

LimboUtility::~LimboUtility() {
	singleton = nullptr;
}
