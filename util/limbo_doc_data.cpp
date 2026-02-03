/**
 * limbo_doc_data.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef LIMBOAI_GDEXTENSION
#ifdef TOOLS_ENABLED

#include "limbo_doc_data.h"

void LimboDocData::register_doc_entry(const DocEntry &p_entry) {
	get_raw_entries().push_back(p_entry);
}

void LimboDocData::ensure_initialized() {
	if (is_initialized()) {
		return;
	}
	is_initialized() = true;

	HashMap<String, ClassDoc> &class_docs = get_class_docs();
	std::vector<DocEntry> &raw_entries = get_raw_entries();

	for (size_t i = 0; i < raw_entries.size(); i++) {
		const DocEntry &entry = raw_entries[i];
		if (entry.name == nullptr || entry.name[0] == '\0') {
			continue;
		}

		ClassDoc doc;
		doc.name = String(entry.name);
		doc.brief_description = entry.brief_description ? String(entry.brief_description) : String();
		doc.description = entry.description ? String(entry.description) : String();

		class_docs[doc.name] = doc;
	}
}

const LimboDocData::ClassDoc *LimboDocData::get_class_doc(const String &p_class_or_script_path) {
	ensure_initialized();

	HashMap<String, ClassDoc> &class_docs = get_class_docs();

	if (p_class_or_script_path.begins_with("res://")) {
		// Try to find by script path (with quotes, as stored by Godot's doc system).
		String path_key = "\"" + p_class_or_script_path.trim_prefix("res://") + "\"";
		HashMap<String, ClassDoc>::Iterator it = class_docs.find(path_key);
		if (it != class_docs.end()) {
			return &it->value;
		}

		// Try to find by guessed class name from filename.
		String maybe_class_name = p_class_or_script_path.get_file().get_basename().to_pascal_case();
		it = class_docs.find(maybe_class_name);
		if (it != class_docs.end()) {
			return &it->value;
		}

		return nullptr;
	}

	// Try to find by class name directly.
	HashMap<String, ClassDoc>::Iterator it = class_docs.find(p_class_or_script_path);
	if (it != class_docs.end()) {
		return &it->value;
	}

	return nullptr;
}

String LimboDocData::get_class_description(const String &p_class_or_script_path) {
	const ClassDoc *doc = get_class_doc(p_class_or_script_path);
	if (doc == nullptr) {
		return String();
	}

	if (doc->description.is_empty()) {
		return doc->brief_description;
	}
	return doc->description;
}

void LimboDocData::clear() {
	get_class_docs().clear();
	get_raw_entries().clear();
	is_initialized() = false;
}

#endif // TOOLS_ENABLED
#endif // LIMBOAI_GDEXTENSION