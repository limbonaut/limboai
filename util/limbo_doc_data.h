/**
 * limbo_doc_data.h
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef LIMBO_DOC_DATA_H
#define LIMBO_DOC_DATA_H

#ifdef LIMBOAI_GDEXTENSION
#ifdef TOOLS_ENABLED

#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/string.hpp>

#include <vector>

using namespace godot;

// Stores class documentation data parsed from embedded XML for GDExtension.
// This allows us to show help tooltips in the editor even though EditorHelp
// and EditorHelpBitTooltip are not exposed in the Godot API.
//
// Documentation is registered via static initializers from the generated
// doc_lookup.gen.cpp file using raw C strings to avoid initialization order issues.
class LimboDocData {
public:
	// Raw doc entry using const char* for safe static initialization.
	struct DocEntry {
		const char *name;
		const char *brief_description;
		const char *description;
	};

	// Converted doc with Godot Strings for runtime use.
	struct ClassDoc {
		String name;
		String brief_description;
		String description;
	};

private:
	// Use std::vector for raw entries to avoid godot-cpp initialization issues.
	static std::vector<DocEntry> &get_raw_entries() {
		static std::vector<DocEntry> entries;
		return entries;
	}

	static HashMap<String, ClassDoc> &get_class_docs() {
		static HashMap<String, ClassDoc> class_docs;
		return class_docs;
	}

	static bool &is_initialized() {
		static bool initialized = false;
		return initialized;
	}

	// Convert raw entries to ClassDoc and populate the lookup map.
	static void ensure_initialized();

public:
	// Register a raw documentation entry. Safe to call from static initializers.
	static void register_doc_entry(const DocEntry &p_entry);

	// Get class documentation by class name or script path.
	// For script paths (starting with "res://"), tries to find by path or by guessed class name.
	static const ClassDoc *get_class_doc(const String &p_class_or_script_path);

	// Get description for a class. Returns brief_description if description is empty.
	// Returns empty string if class is not found.
	static String get_class_description(const String &p_class_or_script_path);

	// Clear all registered documentation. Called during editor deinitialization.
	static void clear();
};

#endif // TOOLS_ENABLED
#endif // LIMBOAI_GDEXTENSION

#endif // LIMBO_DOC_DATA_H