/* bt_cooldown.h */

#ifndef BT_COOLDOWN_H
#define BT_COOLDOWN_H

#include "bt_decorator.h"
#include "core/object/object.h"
#include "scene/main/scene_tree.h"

class BTCooldown : public BTDecorator {
	GDCLASS(BTCooldown, BTDecorator);

private:
	double duration = 10.0;
	bool process_pause = false;
	bool start_cooled = false;
	bool trigger_on_failure = false;
	String cooldown_state_var = "";

	Ref<SceneTreeTimer> timer = nullptr;

	void _chill();
	void _on_timeout();

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual void _setup() override;
	virtual int _tick(double p_delta) override;

public:
	void set_duration(double p_value) {
		duration = p_value;
		emit_changed();
	}
	double get_duration() const { return duration; }
	void set_process_pause(bool p_value) {
		process_pause = p_value;
		emit_changed();
	}
	bool get_process_pause() const { return process_pause; }
	void set_start_cooled(bool p_value) {
		start_cooled = p_value;
		emit_changed();
	}
	bool get_start_cooled() const { return start_cooled; }
	void set_trigger_on_failure(bool p_value) {
		trigger_on_failure = p_value;
		emit_changed();
	}
	bool get_trigger_on_failure() const { return trigger_on_failure; }
	void set_cooldown_state_var(String p_value) {
		cooldown_state_var = p_value;
		emit_changed();
	}
	String get_cooldown_state_var() const { return cooldown_state_var; }
};

#endif // BT_COOLDOWN_H