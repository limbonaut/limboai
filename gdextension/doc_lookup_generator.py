#!/usr/bin/env python
"""
doc_lookup_generator.py
==============================================================================
Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.

Use of this source code is governed by an MIT-style
license that can be found in the LICENSE file or at
https://opensource.org/licenses/MIT.
==============================================================================

Generates C++ code that registers class documentation for the LimboDocData system.
This allows GDExtension builds to display help tooltips for task classes.
"""

import glob
import os
import re
import xml.etree.ElementTree as ET


def escape_cpp_string(s):
    """Escape a string for use in C++ source code."""
    if not s:
        return ""
    # Escape backslashes first, then quotes, then handle special chars
    s = s.replace("\\", "\\\\")
    s = s.replace('"', '\\"')
    s = s.replace("\n", "\\n")
    s = s.replace("\r", "")
    s = s.replace("\t", "\\t")
    return s


def strip_bbcode_for_brief(text):
    """
    Strip BBCode tags for brief descriptions while preserving readability.
    Keeps the text content but removes formatting tags.
    """
    if not text:
        return ""

    # Replace [code]...[/code] with backticks or just the content
    text = re.sub(r"\[code\](.*?)\[/code\]", r"\1", text)

    # Replace [b]...[/b] and [i]...[/i] - keep content
    text = re.sub(r"\[b\](.*?)\[/b\]", r"\1", text)
    text = re.sub(r"\[i\](.*?)\[/i\]", r"\1", text)

    # Replace [ClassName] references - keep as is for now
    # These will be displayed as plain text in tooltips

    # Remove other BBCode tags but keep content
    text = re.sub(r"\[url=.*?\](.*?)\[/url\]", r"\1", text)
    text = re.sub(r"\[param\s+(.*?)\]", r"\1", text)
    text = re.sub(r"\[member\s+(.*?)\]", r"\1", text)
    text = re.sub(r"\[method\s+(.*?)\]", r"\1", text)
    text = re.sub(r"\[signal\s+(.*?)\]", r"\1", text)
    text = re.sub(r"\[constant\s+(.*?)\]", r"\1", text)
    text = re.sub(r"\[enum\s+(.*?)\]", r"\1", text)
    text = re.sub(r"\[annotation\s+(.*?)\]", r"\1", text)
    text = re.sub(r"\[theme_item\s+(.*?)\]", r"\1", text)

    # Clean up whitespace
    text = re.sub(r"\s+", " ", text).strip()

    return text


def parse_xml_doc(xml_path):
    """
    Parse a documentation XML file and extract class info.
    Returns a dict with name, brief_description, and description.
    """
    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()

        if root.tag != "class":
            return None

        class_name = root.get("name", "")
        if not class_name:
            return None

        brief_desc = ""
        description = ""

        brief_elem = root.find("brief_description")
        if brief_elem is not None and brief_elem.text:
            brief_desc = brief_elem.text.strip()

        desc_elem = root.find("description")
        if desc_elem is not None and desc_elem.text:
            description = desc_elem.text.strip()

        return {
            "name": class_name,
            "brief_description": brief_desc,
            "description": description,
        }
    except ET.ParseError as e:
        print(f"Warning: Failed to parse {xml_path}: {e}")
        return None
    except Exception as e:
        print(f"Warning: Error processing {xml_path}: {e}")
        return None


def generate_doc_lookup_source(dst, source):
    """
    Generate C++ source file with documentation registration calls.

    Args:
        dst: Output file path
        source: List of XML source files or paths
    """
    docs = []

    for src in source:
        src_path = str(src)
        if not src_path.endswith(".xml"):
            continue

        doc = parse_xml_doc(src_path)
        if doc and doc["name"]:
            docs.append(doc)

    # Sort by class name for consistent output
    docs.sort(key=lambda x: x["name"])

    with open(dst, "w", encoding="utf-8") as f:
        f.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
        f.write("\n")
        f.write("#ifdef LIMBOAI_GDEXTENSION\n")
        f.write("#ifdef TOOLS_ENABLED\n")
        f.write("\n")
        f.write('#include "../util/limbo_doc_data.h"\n')
        f.write("\n")
        f.write("namespace {\n")
        f.write("\n")
        f.write("// Auto-generated documentation registration\n")
        f.write("// Uses const char* to avoid String initialization order issues.\n")
        f.write("struct LimboDocDataInit {\n")
        f.write("\tLimboDocDataInit() {\n")

        for doc in docs:
            name = escape_cpp_string(doc["name"])
            # For tooltips, we prefer brief description as it's more concise
            # Fall back to full description if brief is empty
            brief = escape_cpp_string(strip_bbcode_for_brief(doc["brief_description"]))
            desc = escape_cpp_string(strip_bbcode_for_brief(doc["description"]))

            f.write(f'\t\tLimboDocData::register_doc_entry({{ "{name}", "{brief}", "{desc}" }});\n')

        f.write("\t}\n")
        f.write("} _limbo_doc_data_init;\n")
        f.write("\n")
        f.write("} // namespace\n")
        f.write("\n")
        f.write("#endif // TOOLS_ENABLED\n")
        f.write("#endif // LIMBOAI_GDEXTENSION\n")

    print(f"Generated documentation lookup with {len(docs)} classes: {dst}")


def scons_generate_doc_lookup_source(target, source, env):
    """SCons builder action for generating doc lookup source."""
    generate_doc_lookup_source(str(target[0]), source)


def generate_doc_lookup_source_from_directory(target, directory):
    """
    Generate doc lookup source from all XML files in a directory.

    Args:
        target: Output file path
        directory: Directory containing XML documentation files
    """
    xml_files = glob.glob(os.path.join(directory, "*.xml"))
    generate_doc_lookup_source(target, xml_files)


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 3:
        print("Usage: doc_lookup_generator.py <output_file> <doc_classes_dir>")
        print("Example: doc_lookup_generator.py gen/doc_lookup.gen.cpp doc_classes/")
        sys.exit(1)

    output_file = sys.argv[1]
    doc_dir = sys.argv[2]

    if not os.path.isdir(doc_dir):
        print(f"Error: Directory not found: {doc_dir}")
        sys.exit(1)

    # Ensure output directory exists
    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)

    generate_doc_lookup_source_from_directory(output_file, doc_dir)
