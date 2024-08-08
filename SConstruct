#!/usr/bin/env python
import os
import sys

# This is SConstruct file for building GDExtension variant using SCONS build system.
# For module variant, see SCsub file.

# Use --project=DIR to customize output path for built targets.
# - Built targets are placed into "DIR/addons/limboai/bin".
# - For example: scons --project="../my_project"
#   - built targets will be placed into "../my_project/addons/limboai/bin".
# - If not specified, built targets are put into the demo/ project.

AddOption(
    "--project",
    dest="project",
    type="string",
    nargs=1,
    action="store",
    metavar="DIR",
    default="demo",
    help="Specify project directory",
)

help_text = """
Options:
  --project=DIR     Specify project directory (default: "demo");
                    built targets will be placed in DIR/addons/limboai/bin
"""
Help(help_text)

project_dir = GetOption("project")
if not os.path.isdir(project_dir):
    print("Project directory not found: " + project_dir)
    Exit(1)

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# Generate version header.
from limboai_version import generate_module_version_header

print("Generating LimboAI version header...")
generate_module_version_header()

# Update icon entries in limboai.gdextension file.
# Note: This will remove everything after [icons] section, and rebuild it with generated icon entries.
sys.path.append("gdextension")
from update_icon_entries import update_icon_entries

print("Updating LimboAI icon entries...")
update_icon_entries(silent=True)
sys.path.remove("gdextension")

# Tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPDEFINES=["LIMBOAI_GDEXTENSION"])
sources = Glob("*.cpp")
sources += Glob("blackboard/*.cpp")
sources += Glob("blackboard/bb_param/*.cpp")
sources += Glob("bt/*.cpp")
sources += Glob("bt/tasks/*.cpp")
sources += Glob("bt/tasks/blackboard/*.cpp")
sources += Glob("bt/tasks/composites/*.cpp")
sources += Glob("bt/tasks/decorators/*.cpp")
sources += Glob("bt/tasks/scene/*.cpp")
sources += Glob("bt/tasks/utility/*.cpp")
sources += Glob("gdextension/*.cpp")
sources += Glob("editor/debugger/*.cpp")
sources += Glob("editor/*.cpp")
sources += Glob("hsm/*.cpp")
sources += Glob("util/*.cpp")

# Generate documentation header.
if env["target"] in ["editor", "template_debug"]:
    doc_data = env.GodotCPPDocData("gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
    sources.append(doc_data)

# Build library.
if env["platform"] == "macos":
    library = env.SharedLibrary(
        project_dir
        + "/addons/limboai/bin/liblimboai.{}.{}.framework/liblimboai.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        project_dir + "/addons/limboai/bin/liblimboai{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

# Deploy limboai.gdextension into PROJECT/addons/limboai/bin.
deploy_manifest = env.Command(
    project_dir + "/addons/limboai/bin/limboai.gdextension",
    "gdextension/limboai.gdextension",
    Copy("$TARGET", "$SOURCE"),
)

Default(library, deploy_manifest)
