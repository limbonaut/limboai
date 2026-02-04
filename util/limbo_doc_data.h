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

using namespace godot;

// Accessor function defined in doc_data.gen.cpp to retrieve compressed XML data.
extern void limbo_doc_data_load(const unsigned char **r_data, int *r_compressed_size, int *r_uncompressed_size);

// Stores class documentation data parsed from embedded XML for GDExtension.
// This allows us to show help tooltips in the editor even though EditorHelp
// and EditorHelpBitTooltip are not exposed in the Godot API.
//
// Documentation is parsed from compressed XML data in doc_data.gen.cpp via the
// limbo_doc_data_load() accessor function, avoiding data duplication.
class LimboDocData {
public:
	// Class documentation with Godot Strings for runtime use.
	struct ClassDoc {
		String name;
		String brief_description;
		String description;
	};

private:
	static HashMap<String, ClassDoc> &get_class_docs() {
		static HashMap<String, ClassDoc> class_docs;
		return class_docs;
	}

	static bool &is_initialized() {
		static bool initialized = false;
		return initialized;
	}

	// Initialize from compressed XML data and populate the lookup map.
	static void ensure_initialized();

	// Parse XML string and extract class documentation.
	static void parse_xml_docs(const String &p_xml);

	// Strip BBCode tags for brief descriptions while preserving readability.
	static String strip_bbcode(const String &p_text);

public:
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