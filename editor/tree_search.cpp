/**
 * tree_search.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#include "tree_search.h"

#include "../compat/editor_scale.h"
#include "../compat/translation.h"
#include "../util/limbo_string_names.h"
#include "../util/limbo_utility.h"

#ifdef LIMBOAI_MODULE
#include "scene/resources/font.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#endif // LIMBOAI_GDEXTENSION

#define UPPER_BOUND (1 << 15) // for substring search.

/* ------- TreeSearch ------- */

void TreeSearch::_clean_callable_cache() {
	ERR_FAIL_COND(!tree_reference);

	HashMap<TreeItem *, Callable> new_callable_cache;
	new_callable_cache.reserve(callable_cache.size());

	for (int i = 0; i < ordered_tree_items.size(); i++) {
		TreeItem *cur_item = ordered_tree_items[i];
		if (callable_cache.has(cur_item)) {
			new_callable_cache[cur_item] = callable_cache[cur_item];
		}
	}
	callable_cache = new_callable_cache;
}

void TreeSearch::_filter_tree() {
	ERR_FAIL_COND(!tree_reference);
	if (!tree_reference->get_root()) {
		return;
	}
	if (matching_entries.is_empty()) {
		return;
	}

	_filter_tree(tree_reference->get_root(), false);
}

void TreeSearch::_filter_tree(TreeItem *p_item, bool p_parent_matching) {
	bool visible = (number_matches.has(p_item) && (number_matches.get(p_item) > 0)) || p_parent_matching;

	p_item->set_visible(visible);

	bool is_matching = _vector_has_bsearch(matching_entries, p_item);
	for (int i = 0; i < p_item->get_child_count(); i++) {
		_filter_tree(p_item->get_child(i), is_matching | p_parent_matching);
	}
}

// Makes all tree items visible.
void TreeSearch::_clear_filter() {
	ERR_FAIL_COND(!tree_reference);
	if (!tree_reference->get_root()) {
		return;
	}

	Vector<TreeItem *> items = { tree_reference->get_root() };
	for (int idx = 0; idx < items.size(); idx++) {
		TreeItem *cur_item = items[idx];
		cur_item->set_visible(true);

		for (int i = 0; i < cur_item->get_child_count(); i++) {
			items.push_back(cur_item->get_child(i));
		}
	}
}

void TreeSearch::_highlight_tree() {
	ERR_FAIL_COND(!tree_reference);

	for (HashMap<TreeItem *, int>::Iterator it = number_matches.begin(); it != number_matches.end(); ++it) {
		TreeItem *tree_item = it->key;
		_highlight_tree_item(tree_item);
	}
	tree_reference->queue_redraw();
}

void TreeSearch::_highlight_tree_item(TreeItem *p_tree_item) {
	int num_m = number_matches.has(p_tree_item) ? number_matches.get(p_tree_item) : 0;

	if (num_m == 0) {
		return;
	}

	// Make sure to also call any draw method already defined.
	Callable parent_draw_method;
	if (p_tree_item->get_cell_mode(0) == TreeItem::CELL_MODE_CUSTOM) {
		parent_draw_method = p_tree_item->get_custom_draw_callback(0);
	}

	// If the cached draw method is already applied, do nothing.
	if (callable_cache.has(p_tree_item) && parent_draw_method == callable_cache.get(p_tree_item)) {
		return;
	}

	Callable draw_callback = callable_mp(this, &TreeSearch::_draw_highlight_item).bind(parent_draw_method);
	callable_cache[p_tree_item] = draw_callback;

	// This is necessary because of the modularity of this implementation.
	// Cache render properties of entry.
	String cached_text = p_tree_item->get_text(0);
	Ref<Texture2D> cached_icon = p_tree_item->get_icon(0);
	int cached_max_width = p_tree_item->get_icon_max_width(0);

	// This removes render properties in entry.
	p_tree_item->set_custom_draw_callback(0, draw_callback);
	p_tree_item->set_cell_mode(0, TreeItem::CELL_MODE_CUSTOM);

	// Restore render properties.
	p_tree_item->set_text(0, cached_text);
	p_tree_item->set_icon(0, cached_icon);
	p_tree_item->set_icon_max_width(0, cached_max_width);
}

// Custom draw callback for highlighting (bind the parent_draw_method to this)
void TreeSearch::_draw_highlight_item(TreeItem *p_tree_item, const Rect2 p_rect, const Callable &p_parent_draw_method) {
	if (!p_tree_item) {
		return;
	}

	// Call any parent draw methods such as for probability FIRST.
	p_parent_draw_method.call(p_tree_item, p_rect);

	// First part: outline
	if (matching_entries.has(p_tree_item)) {
		// Font info
		Ref<Font> font = p_tree_item->get_custom_font(0);
		if (font.is_null()) {
			font = p_tree_item->get_tree()->get_theme_font(LW_NAME(font));
		}
		ERR_FAIL_COND(font.is_null());
		float font_size = p_tree_item->get_custom_font_size(0);
		if (font_size == -1) {
			font_size = p_tree_item->get_tree()->get_theme_font_size(LW_NAME(font));
		}

		// Substring size
		String string_full = p_tree_item->get_text(0);
		StringSearchIndices substring_idx = _substring_bounds(string_full, _get_search_mask());

		String substring_match = string_full.substr(substring_idx.lower, substring_idx.upper - substring_idx.lower);
		Vector2 substring_match_size = font->get_string_size(substring_match, HORIZONTAL_ALIGNMENT_LEFT, -1.f, font_size);

		String substring_before = string_full.substr(0, substring_idx.lower);
		Vector2 substring_before_size = font->get_string_size(substring_before, HORIZONTAL_ALIGNMENT_LEFT, -1.f, font_size);

		// Stylebox
		Ref<StyleBox> stylebox = p_tree_item->get_tree()->get_theme_stylebox(LW_NAME(Focus));
		ERR_FAIL_COND(stylebox.is_null());

		// Extract separation
		float h_sep = p_tree_item->get_tree()->get_theme_constant(LW_NAME(h_separation));

		// Compose draw rect
		const Vector2 PADDING = Vector2(4., 2.);
		Rect2 draw_rect = p_rect;

		Vector2 rect_offset = Vector2(substring_before_size.x, 0);
		rect_offset.x += p_tree_item->get_icon_max_width(0);
		rect_offset.x += (h_sep + 4. * EDSCALE);
		rect_offset.y = (p_rect.size.y - substring_match_size.y) / 2; // center box vertically

		draw_rect.position += rect_offset - PADDING / 2;
		draw_rect.size = substring_match_size + PADDING;

		// Draw
		stylebox->draw(p_tree_item->get_tree()->get_canvas_item(), draw_rect);
	}

	// Second part: draw number
	int num_mat = number_matches.has(p_tree_item) ? number_matches.get(p_tree_item) : 0;
	if (num_mat > 0) {
		float h_sep = p_tree_item->get_tree()->get_theme_constant(LW_NAME(h_separation));
		Ref<Font> font = tree_reference->get_theme_font(LW_NAME(font));
		float font_size = tree_reference->get_theme_font_size(LW_NAME(font)) * 0.75;

		String num_string = String::num_int64(num_mat);
		Vector2 string_size = font->get_string_size(num_string, HORIZONTAL_ALIGNMENT_CENTER, -1, font_size);
		Vector2 text_pos = p_rect.position;

		text_pos.x += p_rect.size.x - string_size.x - h_sep;
		text_pos.y += font->get_descent(font_size) + p_rect.size.y / 2.; // center vertically

		font->draw_string(tree_reference->get_canvas_item(), text_pos, num_string, HORIZONTAL_ALIGNMENT_CENTER, -1, font_size);
	}
}

void TreeSearch::_update_matching_entries(const String &p_search_mask) {
	Vector<TreeItem *> accum;
	_find_matching_entries(tree_reference->get_root(), p_search_mask, accum);
	matching_entries = accum;
}

/* Linaerizes the tree into [ordered_tree_items] like so:
 - i1
	- i2
	- i3
 - i4 ---> [i1,i2,i3,i4]
*/
void TreeSearch::_update_ordered_tree_items(TreeItem *p_tree_item) {
	if (!p_tree_item) {
		return;
	}
	if (p_tree_item == p_tree_item->get_tree()->get_root()) {
		ordered_tree_items.clear();
	}
	// Add the current item to the list.
	ordered_tree_items.push_back(p_tree_item);

	// Recursively collect items from the first child.
	TreeItem *child = p_tree_item->get_first_child();
	while (child) {
		_update_ordered_tree_items(child);
		child = child->get_next();
	}
}

void TreeSearch::_update_number_matches() {
	ERR_FAIL_COND(!tree_reference);
	number_matches.clear();
	number_matches.reserve(ordered_tree_items.size());

	TreeItem *tree_root = tree_reference->get_root();
	if (!tree_root) {
		return;
	}
	_update_number_matches(tree_root);
}

void TreeSearch::_update_number_matches(TreeItem *item) {
	ERR_FAIL_COND(!item);
	for (int i = 0; i < item->get_child_count(); i++) {
		TreeItem *child = item->get_child(i);
		_update_number_matches(child);
	}
	int count = _vector_has_bsearch(matching_entries, item) ? 1 : 0;

	for (int i = 0; i < item->get_child_count(); i++) {
		TreeItem *child = item->get_child(i);
		count += number_matches.has(child) ? number_matches.get(child) : 0;
	}
	if (count == 0) {
		return;
	}

	number_matches[item] = count;
}

String TreeSearch::_get_search_mask() const {
	ERR_FAIL_COND_V(!search_panel, "");
	return search_panel->get_text();
}

void TreeSearch::_find_matching_entries(TreeItem *p_tree_item, const String &p_search_mask, Vector<TreeItem *> &p_accum) const {
	if (!p_tree_item) {
		return;
	}

	StringSearchIndices item_search_indices = _substring_bounds(p_tree_item->get_text(0), p_search_mask);
	if (item_search_indices.hit()) {
		p_accum.push_back(p_tree_item);
	}

	for (int i = 0; i < p_tree_item->get_child_count(); i++) {
		TreeItem *child = p_tree_item->get_child(i);
		_find_matching_entries(child, p_search_mask, p_accum);
	}

	// Sort the result if we are at the root.
	if (p_tree_item == p_tree_item->get_tree()->get_root()) {
		p_accum.sort();
	}

	return;
}

// Returns the lower and upper bounds of a substring. Does fuzzy search: Simply looks if words exist in right ordering.
// Also ignores case if p_search_mask is lowercase. Example:
// p_searcheable = "TimeLimit 2 sec", p_search_mask = limit 2 sec -> [4,14]. With p_search_mask = "LimiT 2 SEC" or "Limit sec 2" -> [-1,-1]
TreeSearch::StringSearchIndices TreeSearch::_substring_bounds(const String &p_searchable, const String &p_search_mask) const {
	StringSearchIndices result;
	result.lower = UPPER_BOUND;
	result.upper = 0;

	if (p_search_mask.is_empty()) {
		return result; // Early return if search_mask is empty.
	}

	// Determine if the search should be case-insensitive.
	bool is_case_insensitive = (p_search_mask == p_search_mask.to_lower());
	String searchable_processed = is_case_insensitive ? p_searchable.to_lower() : p_searchable;

	PackedStringArray words = p_search_mask.split(" ");
	int word_position = 0;

	for (const String &word : words) {
		if (word.is_empty()) {
			continue; // Skip empty words.
		}

		String word_processed = is_case_insensitive ? word.to_lower() : word;

		// Find the position of the next word in the searchable string.
		word_position = searchable_processed.find(word_processed, word_position);

		if (word_position < 0) {
			// If any word is not found, return an empty StringSearchIndices.
			return StringSearchIndices();
		}

		// Update lower and upper bounds.
		result.lower = MIN(result.lower, word_position);
		result.upper = MAX(result.upper, static_cast<int>(word_position + word.length()));
	}

	return result;
}

void TreeSearch::_select_item(TreeItem *p_item) {
	if (!p_item) {
		return;
	}

	ERR_FAIL_COND(!tree_reference || p_item->get_tree() != tree_reference);

	// First unfold ancestors
	TreeItem *ancestor = p_item->get_parent();
	while (ancestor) {
		ancestor->set_collapsed(false);
		ancestor = ancestor->get_parent();
	}
	// Then scroll to [item]
	tree_reference->scroll_to_item(p_item);

	// ...and select it
	tree_reference->deselect_all();
	tree_reference->set_selected(p_item, 0);
}

void TreeSearch::_select_first_match() {
	if (matching_entries.size() == 0) {
		return;
	}
	for (int i = 0; i < ordered_tree_items.size(); i++) {
		TreeItem *item = ordered_tree_items[i];
		if (!_vector_has_bsearch(matching_entries, item)) {
			continue;
		}
		_select_item(item);
		return;
	}
}

void TreeSearch::_select_last_match() {
	if (matching_entries.size() == 0) {
		return;
	}
	for (int i = ordered_tree_items.size() - 1; i >= 0; i--) {
		TreeItem *item = ordered_tree_items[i];
		if (!_vector_has_bsearch(matching_entries, item)) {
			continue;
		}
		_select_item(item);
		return;
	}
}

void TreeSearch::_select_previous_match() {
	if (matching_entries.size() == 0) {
		return;
	}

	TreeItem *selected = tree_reference->get_selected();
	if (!selected) {
		_select_last_match();
		return;
	}
	// Find [selected_idx] among ordered_tree_items.
	int selected_idx = 0;
	for (int i = ordered_tree_items.size() - 1; i >= 0; i--) {
		if (ordered_tree_items[i] == selected) {
			selected_idx = i;
			break;
		}
	}
	// Find first entry before [selected_idx].
	for (int i = MIN(ordered_tree_items.size() - 1, selected_idx) - 1; i >= 0; i--) {
		TreeItem *item = ordered_tree_items[i];
		if (_vector_has_bsearch(matching_entries, item)) {
			_select_item(item);
			return;
		}
	}
	// Wrap around.
	_select_last_match();
}

void TreeSearch::_select_next_match() {
	if (matching_entries.size() == 0) {
		return;
	}

	TreeItem *selected = tree_reference->get_selected();
	if (!selected) {
		_select_first_match();
		return;
	}

	// Find [selected_idx] among ordered_tree_items
	int selected_idx = 0;
	for (int i = 0; i < ordered_tree_items.size(); i++) {
		if (ordered_tree_items[i] == selected) {
			selected_idx = i;
			break;
		}
	}

	// Find first entry after [selected_idx].
	for (int i = MAX(0, selected_idx) + 1; i < ordered_tree_items.size(); i++) {
		TreeItem *item = ordered_tree_items[i];
		if (_vector_has_bsearch(matching_entries, item)) {
			_select_item(item);
			return;
		}
	}
	// Wrap around.
	_select_first_match();
}

void TreeSearch::_on_search_panel_closed() {
	if (!tree_reference) {
		return;
	}
	tree_reference->grab_focus();
}

template <typename T>
inline bool TreeSearch::_vector_has_bsearch(Vector<T *> &p_vec, T *element) const {
	int idx = p_vec.bsearch(element, true);
	bool in_array = idx >= 0 && idx < p_vec.size();

	return in_array && p_vec[idx] == element;
}

void TreeSearch::notify_item_edited(TreeItem *item) {
	if (item->get_cell_mode(0) != TreeItem::CELL_MODE_CUSTOM) {
		return;
	}
	_highlight_tree_item(item);
}

// Called as a post-processing step for the already constructed tree.
void TreeSearch::update_search(Tree *p_tree) {
	ERR_FAIL_COND(!search_panel || !p_tree);

	tree_reference = p_tree;

	if (!tree_reference->get_root()) {
		return;
	}

	if (!search_panel->is_visible() || search_panel->get_text().length() == 0) {
		// Clear and redraw if search was active recently.
		if (was_searched_recently) {
			number_matches.clear();
			matching_entries.clear();

			_clear_filter();

			was_searched_recently = false;
			p_tree->queue_redraw();
		}
		return;
	}
	was_searched_recently = true;

	String search_mask = search_panel->get_text();
	TreeSearchMode search_mode = search_panel->get_search_mode();

	_update_ordered_tree_items(p_tree->get_root());
	_update_matching_entries(search_mask);
	_update_number_matches();

	_highlight_tree();
	if (search_mode == TreeSearchMode::FILTER) {
		_filter_tree();
		was_filtered_recently = true;
	} else if (was_filtered_recently) {
		_clear_filter();
		was_filtered_recently = false;
	}
	_clean_callable_cache();
}

TreeSearch::TreeSearch(TreeSearchPanel *p_search_panel) {
	search_panel = p_search_panel;
	search_panel->connect(LW_NAME(text_submitted), callable_mp(this, &TreeSearch::_select_next_match));
	search_panel->connect(LW_NAME(Close), callable_mp(this, &TreeSearch::_on_search_panel_closed));
	search_panel->connect("select_previous_match", callable_mp(this, &TreeSearch::_select_previous_match));
}

/* !TreeSearch */

/* ------- TreeSearchPanel ------- */

void TreeSearchPanel::_add_spacer(float p_width_multiplier) {
	Control *spacer = memnew(Control);
	spacer->set_custom_minimum_size(Vector2(8.0 * EDSCALE * p_width_multiplier, 0.0));
	add_child(spacer);
}

void TreeSearchPanel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			// Close callbacks
			close_button->connect(LW_NAME(pressed), Callable(this, LW_NAME(set_visible)).bind(false));
			close_button->connect(LW_NAME(pressed), Callable(this, LW_NAME(emit_signal)).bind(LW_NAME(Close)));
			close_button->set_shortcut(LW_GET_SHORTCUT("limbo_ai/hide_tree_search")); // TODO: use internal shortcut. also sets tooltip...
			// Search callbacks
			Callable c_update_requested = Callable(this, LW_NAME(emit_signal)).bind("update_requested");
			Callable c_text_submitted = Callable(this, LW_NAME(emit_signal)).bind(LW_NAME(text_submitted));
			Callable c_select_previous_match = Callable(this, LW_NAME(emit_signal)).bind("select_previous_match");
			find_next_button->connect(LW_NAME(pressed), c_text_submitted);
			find_prev_button->connect(LW_NAME(pressed), c_select_previous_match);

			line_edit_search->connect(LW_NAME(text_changed), c_update_requested.unbind(1));
			check_button_filter_highlight->connect(LW_NAME(pressed), c_update_requested);
			line_edit_search->connect(LW_NAME(text_submitted), c_text_submitted.unbind(1));
			break;
		}
		case NOTIFICATION_THEME_CHANGED: {
			close_button->set_button_icon(get_theme_icon(LW_NAME(Close), LW_NAME(EditorIcons)));
			find_prev_button->set_button_icon(get_theme_icon("MoveUp", LW_NAME(EditorIcons)));
			find_next_button->set_button_icon(get_theme_icon("MoveDown", LW_NAME(EditorIcons)));
			label_filter->set_text(TTR("Filter"));
			break;
		}
	}
}

void TreeSearchPanel::_bind_methods() {
	ADD_SIGNAL(MethodInfo("update_requested"));
	ADD_SIGNAL(MethodInfo(LW_NAME(text_submitted)));
	ADD_SIGNAL(MethodInfo("select_previous_match"));
	ADD_SIGNAL(MethodInfo(LW_NAME(Close)));
}

TreeSearchPanel::TreeSearchPanel() {
	line_edit_search = memnew(LineEdit);
	check_button_filter_highlight = memnew(CheckBox);
	close_button = memnew(Button);
	find_next_button = memnew(Button);
	find_prev_button = memnew(Button);
	label_filter = memnew(Label);

	line_edit_search->set_placeholder(TTR("Search tree"));

	close_button->set_theme_type_variation(LW_NAME(FlatButton));
	find_next_button->set_theme_type_variation(LW_NAME(FlatButton));
	find_prev_button->set_theme_type_variation(LW_NAME(FlatButton));

	find_next_button->set_tooltip_text("Next Match");
	find_prev_button->set_tooltip_text("Previous Match");
	line_edit_search->set_tooltip_text("Match case if input contains capital letter.");

	// Positioning and sizing
	set_anchors_and_offsets_preset(LayoutPreset::PRESET_BOTTOM_WIDE);
	set_v_size_flags(SIZE_SHRINK_CENTER); // Do not expand vertically

	line_edit_search->set_h_size_flags(SIZE_EXPAND_FILL);

	_add_spacer(0.1); // -> Otherwise the lineedits expand margin touches the left border.
	add_child(line_edit_search);
	add_child(find_prev_button);
	add_child(find_next_button);
	_add_spacer(0.25);

	add_child(check_button_filter_highlight);
	add_child(label_filter);

	_add_spacer(0.25);
	add_child(close_button);

	set_visible(false);
}

TreeSearch::TreeSearchMode TreeSearchPanel::get_search_mode() const {
	if (!check_button_filter_highlight || !check_button_filter_highlight->is_pressed()) {
		return TreeSearch::TreeSearchMode::HIGHLIGHT;
	}
	return TreeSearch::TreeSearchMode::FILTER;
}

String TreeSearchPanel::get_text() const {
	return line_edit_search->get_text();
}

TreeSearch::SearchInfo TreeSearchPanel::get_search_info() const {
	TreeSearch::SearchInfo result;
	result.search_mask = get_text();
	result.search_mode = get_search_mode();
	result.visible = is_visible();
	return result;
}

void TreeSearchPanel::set_search_info(const TreeSearch::SearchInfo &p_search_info) {
	line_edit_search->set_text(p_search_info.search_mask);
	check_button_filter_highlight->set_pressed(p_search_info.search_mode == TreeSearch::TreeSearchMode::FILTER);
	set_visible(p_search_info.visible);
	emit_signal("update_requested");
}

void TreeSearchPanel::focus_editor() {
	line_edit_search->grab_focus();
}

/* !TreeSearchPanel */

#endif // TOOLS_ENABLED
