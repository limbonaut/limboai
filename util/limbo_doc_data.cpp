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

#include <godot_cpp/classes/xml_parser.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

String LimboDocData::strip_bbcode(const String &p_text) {
	if (p_text.is_empty()) {
		return String();
	}

	String text = p_text;

	// Replace [code]...[/code] - keep content
	while (text.find("[code]") != -1) {
		int start = text.find("[code]");
		int end = text.find("[/code]");
		if (end != -1) {
			String content = text.substr(start + 6, end - start - 6);
			text = text.substr(0, start) + content + text.substr(end + 7);
		} else {
			break;
		}
	}

	// Replace [b]...[/b] and [i]...[/i] - keep content
	while (text.find("[b]") != -1) {
		int start = text.find("[b]");
		int end = text.find("[/b]");
		if (end != -1) {
			String content = text.substr(start + 3, end - start - 3);
			text = text.substr(0, start) + content + text.substr(end + 4);
		} else {
			break;
		}
	}

	while (text.find("[i]") != -1) {
		int start = text.find("[i]");
		int end = text.find("[/i]");
		if (end != -1) {
			String content = text.substr(start + 3, end - start - 3);
			text = text.substr(0, start) + content + text.substr(end + 4);
		} else {
			break;
		}
	}

	// Replace [url=...]...[/url] - keep content
	while (text.find("[url=") != -1) {
		int start = text.find("[url=");
		int url_end = text.find("]", start);
		int end = text.find("[/url]");
		if (url_end != -1 && end != -1) {
			String content = text.substr(url_end + 1, end - url_end - 1);
			text = text.substr(0, start) + content + text.substr(end + 6);
		} else {
			break;
		}
	}

	// Replace [param name] - extract name
	while (text.find("[param ") != -1) {
		int start = text.find("[param ");
		int end = text.find("]", start);
		if (end != -1) {
			String content = text.substr(start + 7, end - start - 7);
			text = text.substr(0, start) + content + text.substr(end + 1);
		} else {
			break;
		}
	}

	// Replace [member name] - extract name
	while (text.find("[member ") != -1) {
		int start = text.find("[member ");
		int end = text.find("]", start);
		if (end != -1) {
			String content = text.substr(start + 8, end - start - 8);
			text = text.substr(0, start) + content + text.substr(end + 1);
		} else {
			break;
		}
	}

	// Replace [method name] - extract name
	while (text.find("[method ") != -1) {
		int start = text.find("[method ");
		int end = text.find("]", start);
		if (end != -1) {
			String content = text.substr(start + 8, end - start - 8);
			text = text.substr(0, start) + content + text.substr(end + 1);
		} else {
			break;
		}
	}

	// Replace [signal name] - extract name
	while (text.find("[signal ") != -1) {
		int start = text.find("[signal ");
		int end = text.find("]", start);
		if (end != -1) {
			String content = text.substr(start + 8, end - start - 8);
			text = text.substr(0, start) + content + text.substr(end + 1);
		} else {
			break;
		}
	}

	// Replace [constant name] - extract name
	while (text.find("[constant ") != -1) {
		int start = text.find("[constant ");
		int end = text.find("]", start);
		if (end != -1) {
			String content = text.substr(start + 10, end - start - 10);
			text = text.substr(0, start) + content + text.substr(end + 1);
		} else {
			break;
		}
	}

	// Replace [enum name] - extract name
	while (text.find("[enum ") != -1) {
		int start = text.find("[enum ");
		int end = text.find("]", start);
		if (end != -1) {
			String content = text.substr(start + 6, end - start - 6);
			text = text.substr(0, start) + content + text.substr(end + 1);
		} else {
			break;
		}
	}

	// Replace [annotation name] - extract name
	while (text.find("[annotation ") != -1) {
		int start = text.find("[annotation ");
		int end = text.find("]", start);
		if (end != -1) {
			String content = text.substr(start + 12, end - start - 12);
			text = text.substr(0, start) + content + text.substr(end + 1);
		} else {
			break;
		}
	}

	// Replace [theme_item name] - extract name
	while (text.find("[theme_item ") != -1) {
		int start = text.find("[theme_item ");
		int end = text.find("]", start);
		if (end != -1) {
			String content = text.substr(start + 12, end - start - 12);
			text = text.substr(0, start) + content + text.substr(end + 1);
		} else {
			break;
		}
	}

	// Clean up extra whitespace
	text = text.strip_edges();

	return text;
}

void LimboDocData::parse_xml_docs(const String &p_xml) {
	// The XML content is a concatenation of multiple <class> elements.
	// We need to parse each class element and extract name, brief_description, and description.

	HashMap<String, ClassDoc> &class_docs = get_class_docs();

	// Simple parsing approach: find each <class> element and extract data
	int search_pos = 0;
	while (true) {
		int class_start = p_xml.find("<class", search_pos);
		if (class_start == -1) {
			break;
		}

		// Find the closing tag for this class
		int class_end = p_xml.find("</class>", class_start);
		if (class_end == -1) {
			break;
		}
		class_end += 8; // Include "</class>"

		String class_xml = p_xml.substr(class_start, class_end - class_start);

		// Extract class name from the <class name="..."> attribute
		int name_attr_start = class_xml.find("name=\"");
		if (name_attr_start != -1) {
			name_attr_start += 6; // Skip 'name="'
			int name_attr_end = class_xml.find("\"", name_attr_start);
			if (name_attr_end != -1) {
				String class_name = class_xml.substr(name_attr_start, name_attr_end - name_attr_start);

				ClassDoc doc;
				doc.name = class_name;

				// Extract brief_description (keep raw BBCode for proper formatting).
				int brief_start = class_xml.find("<brief_description>");
				if (brief_start != -1) {
					brief_start += 19; // Skip "<brief_description>"
					int brief_end = class_xml.find("</brief_description>");
					if (brief_end != -1) {
						doc.brief_description = class_xml.substr(brief_start, brief_end - brief_start).strip_edges();
					}
				}

				// Extract description (keep raw BBCode for proper formatting).
				int desc_start = class_xml.find("<description>");
				if (desc_start != -1) {
					desc_start += 13; // Skip "<description>"
					int desc_end = class_xml.find("</description>");
					if (desc_end != -1) {
						doc.description = class_xml.substr(desc_start, desc_end - desc_start).strip_edges();
					}
				}

				if (!class_name.is_empty()) {
					class_docs[class_name] = doc;
				}
			}
		}

		search_pos = class_end;
	}
}

void LimboDocData::ensure_initialized() {
	if (is_initialized()) {
		return;
	}
	is_initialized() = true;

	// Get compressed XML data from doc_data.gen.cpp
	const unsigned char *data = nullptr;
	int compressed_size = 0;
	int uncompressed_size = 0;

	limbo_doc_data_load(&data, &compressed_size, &uncompressed_size);

	if (data == nullptr || compressed_size <= 0 || uncompressed_size <= 0) {
		return;
	}

	// Decompress the data
	PackedByteArray compressed;
	compressed.resize(compressed_size);
	memcpy(compressed.ptrw(), data, compressed_size);

	// FileAccess::COMPRESSION_DEFLATE = 1
	PackedByteArray decompressed = compressed.decompress(uncompressed_size, 1);

	if (decompressed.size() == 0) {
		return;
	}

	// Convert to String and parse XML
	String xml_content;
	xml_content.parse_utf8(reinterpret_cast<const char *>(decompressed.ptr()), decompressed.size());

	parse_xml_docs(xml_content);
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
	is_initialized() = false;
}

#endif // TOOLS_ENABLED
#endif // LIMBOAI_GDEXTENSION