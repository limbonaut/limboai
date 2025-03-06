/**
 * callable_fallback_chain.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#ifndef CALLABLE_FALLBACK_CHAIN_H
#define CALLABLE_FALLBACK_CHAIN_H

#include "core/templates/list.h"
#include "core/variant/callable.h"

class CallableFallbackChain {
private:
	mutable List<Callable> chain;

public:
	void push(const Callable &p_callable);
	Callable get_most_recent_valid() const;
};

#endif // CALLABLE_FALLBACK_CHAIN_H
