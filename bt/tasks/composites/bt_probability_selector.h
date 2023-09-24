/**
 * bt_probability_selector.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_PROBABILITY_SELECTOR_H
#define BT_PROBABILITY_SELECTOR_H

#include "modules/limboai/bt/tasks/bt_composite.h"

#include "core/typedefs.h"

class BTProbabilitySelector : public BTComposite {
	GDCLASS(BTProbabilitySelector, BTComposite);
	TASK_CATEGORY(Composites);

private:
	HashSet<Ref<BTTask>> failed_tasks;
	Ref<BTTask> selected_task;
	bool abort_on_failure = false;

	void _select_task();

	_FORCE_INLINE_ double _get_weight(int p_index) const { return get_child(p_index)->get_meta(SNAME("_weight_"), 1.0); }
	_FORCE_INLINE_ double _get_weight(Ref<BTTask> p_task) const { return p_task->get_meta(SNAME("_weight_"), 1.0); }
	_FORCE_INLINE_ void _set_weight(int p_index, double p_weight) { get_child(p_index)->set_meta(SNAME("_weight_"), Variant(p_weight)); }
	_FORCE_INLINE_ double _get_total_weight() const {
		double total = 0.0;
		for (int i = 0; i < get_child_count(); i++) {
			total += _get_weight(i);
		}
		return total;
	}

protected:
	static void _bind_methods();

	virtual void _enter() override;
	virtual void _exit() override;
	virtual Status _tick(double p_delta) override;

public:
	double get_weight(int p_index) const;
	void set_weight(int p_index, double p_weight);

	double get_probability(int p_index) const;
	void set_probability(int p_index, double p_probability);

	void set_abort_on_failure(bool p_abort_on_failure);
	bool get_abort_on_failure() const;
};

#endif // BT_PROBABILITY_SELECTOR_H
