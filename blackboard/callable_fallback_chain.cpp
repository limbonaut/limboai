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
	chain.push_front(p_callable);
}

Callable CallableFallbackChain::get_most_recent_valid() const {
	while (!chain.is_empty()) {
		const Callable &callable = chain.front()->get();
		if (callable.is_valid()) {
			return callable;
		} else {
			chain.pop_front();
		}
	}
	return Callable();
}
