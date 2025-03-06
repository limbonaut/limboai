/**
 * callable_fallback_chain.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "callable_fallback_chain.h"

void CallableFallbackChain::push(const Callable &p_callable) {
	ERR_FAIL_COND(p_callable.is_null());

	List<Callable>::Element *it = chain.back();
	while (it) {
		List<Callable>::Element *cur = it;
		it = it->prev();

		if (cur->get().is_null() || cur->get() == p_callable) {
			cur->erase();
		}
	}

	chain.push_back(p_callable);
}

Callable CallableFallbackChain::get_most_recent_valid() const {
	while (!chain.is_empty()) {
		const Callable &callable = chain.back()->get();
		if (callable.is_valid()) {
			return callable;
		} else {
			chain.pop_back();
		}
	}
	return Callable();
}
