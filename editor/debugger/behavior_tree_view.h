/**
 * behavior_tree_view.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#ifndef BEHAVIOR_TREE_VIEW_H
#define BEHAVIOR_TREE_VIEW_H

#include "behavior_tree_data.h"

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "scene/gui/control.h"
#include "scene/gui/tree.h"
#include "scene/resources/style_box_flat.h"
#include "scene/resources/texture.h"

class BehaviorTreeView : public Control {
	GDCLASS(BehaviorTreeView, Control);

private:
	Tree *tree;

	struct ThemeCache {
		Ref<StyleBoxFlat> sbf_running;
		Ref<StyleBoxFlat> sbf_success;
		Ref<StyleBoxFlat> sbf_failure;

		Ref<Texture2D> icon_running;
		Ref<Texture2D> icon_success;
		Ref<Texture2D> icon_failure;

		Ref<Font> font_custom_name;
	} theme_cache;

	Vector<int> collapsed_ids;

	void _draw_success_status(Object *p_obj, Rect2 p_rect);
	void _draw_running_status(Object *p_obj, Rect2 p_rect);
	void _draw_failure_status(Object *p_obj, Rect2 p_rect);
	void _item_collapsed(Object *p_obj);

protected:
	virtual void _update_theme_item_cache() override;

	static void _bind_methods();

public:
	void update_tree(const BehaviorTreeData &p_data);
	void clear();

	BehaviorTreeView();
};

#endif // BEHAVIOR_TREE_VIEW

#endif // TOOLS_ENABLED