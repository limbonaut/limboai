#pragma once

#include <functional>

#include "core/templates/hashfuncs.h"
#include "core/variant/callable.h"
#include "core/variant/variant.h"

// Used in LimboAI testing to inline signal handlers (for now).
// It only supports zero arguments!
class LambdaCallable : public CallableCustom {
private:
	std::function<void()> lambda_func;
	uint32_t hash_value;

public:
	LambdaCallable(std::function<void()> func) :
			lambda_func(func) {
		hash_value = hash_murmur3_one_64((uint64_t)this);
	}

	virtual uint32_t hash() const override {
		return hash_value;
	}

	virtual String get_as_text() const override {
		return "LambdaCallable";
	}

	virtual CompareEqualFunc get_compare_equal_func() const override {
		return [](const CallableCustom *a, const CallableCustom *b) { return a == b; };
	}

	virtual CompareLessFunc get_compare_less_func() const override {
		return [](const CallableCustom *a, const CallableCustom *b) { return a < b; };
	}

	virtual ObjectID get_object() const override {
		return ObjectID();
	}

	virtual bool is_valid() const override {
		return lambda_func != nullptr; // Critical: Override is_valid()
	}

	virtual int get_argument_count(bool &r_is_valid) const override {
		r_is_valid = true;
		return 0; // Zero arguments only
	}

	virtual void call(const Variant **p_arguments, int p_argcount,
			Variant &r_return_value, Callable::CallError &r_call_error) const override {
		if (p_argcount != 0) {
			r_call_error.error = Callable::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS;
			r_call_error.expected = 0;
			return;
		}

		if (lambda_func) {
			lambda_func();
			r_call_error.error = Callable::CallError::CALL_OK;
		} else {
			r_call_error.error = Callable::CallError::CALL_ERROR_INSTANCE_IS_NULL;
		}
	}
};
