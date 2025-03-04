/**
 * owner_picker.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef OWNER_PICKER_H
#define OWNER_PICKER_H

#ifdef LIMBOAI_MODULE
#include "scene/gui/dialogs.h"
#include "scene/gui/item_list.h"
#endif

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/accept_dialog.hpp>
#include <godot_cpp/classes/item_list.hpp>
using namespace godot;
#endif

class OwnerPicker : public AcceptDialog {
	GDCLASS(OwnerPicker, AcceptDialog);

private:
	ItemList *owners_item_list;

	void _item_activated(int p_item);
	void _selection_confirmed();

protected:
	static void _bind_methods();

	void _notification(int p_what);

	Vector<String> _find_owners(const String &p_path) const;

public:
	void pick_and_open_owner_of_resource(const String &p_path);

	OwnerPicker();
};

#endif // OWNER_PICKER_H
