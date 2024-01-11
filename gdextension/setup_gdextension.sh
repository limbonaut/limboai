#!/bin/bash

## This script creates project structure needed for LimboAI development using GDExtension.
## Works only on Unix-likes. You can still use the directory layout below, if you are on Windows.
##
## Instructions:
## 1) Create the project root directory, name doesn't matter.
## 2) Inside the project root directory, clone the limboai repository:
##    git clone https://github.com/limbonaut/limboai
## 3) From the project root directory, run:
##    bash ./limboai/gdextension/setup_gdextension.sh
##
## Directory layout:
##   project/ -- call this script from here, directory name doesn't matter.
##   project/limboai/ -- LimboAI repository should be here after you clone it.
##   project/godot-cpp/ -- will be created by this script.
##   project/demo/ -- symbolic link that will be created by this script.
##   project/SConstruct -- symbolic link that will be created by this script.
##
## Dependencies: bash, git, python3.

# Script Settings
GODOT_CPP_VERSION=4.2
PYTHON=python

# Colors
HIGHLIGHT_COLOR='\033[1;36m' # Light Cyan
NC='\033[0m' # No Color
ERROR_COLOR="\033[0;31m"  # Red

msg () {  echo -e "$@"; }
highlight() { echo -e "${HIGHLIGHT_COLOR}$@${NC}"; }
error () { echo -e "${ERROR_COLOR}$@${NC}" >&2; }

if [ ! -d "${PWD}/limboai/" ]; then
    error Aborting: \"limboai\" subdirectory is not found.
    msg Tip: Run this script from the project root directory with limboai repository cloned into \"limboai\" subdirectory.
    msg Command: bash ./limboai/gdextension/setup_gdextension.sh
    exit 1
fi

# Interrupt execution and exit on Ctrl-C
trap exit SIGINT

set -e

highlight Setup started.

if [ ! -d "${PWD}/godot-cpp/" ]; then
    highlight -- Cloning godot-cpp...
    git clone -b ${GODOT_CPP_VERSION} https://github.com/godotengine/godot-cpp
    highlight -- Finished cloning godot-cpp.
else
    highlight -- Skipping \"godot-cpp\". Directory already exists!
fi

if [ ! -f "${PWD}/SConstruct" ]; then
    ln -s limboai/gdextension/SConstruct SConstruct
    highlight -- Linked SConstruct.
else
    highlight -- Skipping \"SConstruct\". File already exists!
fi

if [ ! -e "${PWD}/demo" ]; then
    ln -s limboai/demo demo
    highlight -- Linked demo project.
else
    highlight -- Skipping \"demo\". File already exists!
fi

# if [ -d "${PWD}/demo/" ]; then
#     highlight -- Demo project exists. Archiving...
#     backup_version=1
#     backup_dir="${PWD}/demo.old${backup_version}"
#     while [ -d "${backup_dir}" ]; do
#         ((backup_version++))
#         backup_dir="${PWD}/demo.old${backup_version}"
#     done
#     mv demo/ ${backup_dir}
#     highlight -- Demo project archived as \"$(basename ${backup_dir})\".
# fi

# if [ ! -d "${PWD}/demo/" ]; then
#     cp -r limboai/demo demo
#     highlight -- Copied demo project.
# else
#     error Error: \"demo\" directory exists!
#     exit 2
# fi

if [ ! -e "${PWD}/limboai/demo/addons/limboai/bin/limboai.gdextension" ]; then
    ls -l
    mkdir -p ./limboai/demo/addons/limboai/bin/
    cd ./limboai/demo/addons/limboai/bin/
    ln -s ../../../../gdextension/limboai.gdextension limboai.gdextension || ln -s ../../../../limboai/gdextension/limboai.gdextension limboai.gdextension
    ls -l
    cd -
    highlight -- Linked limboai.gdextension.
else
    highlight -- Skipping limboai.gdextension. File already exists!
fi

if [ ! -e "${PWD}/limboai/demo/addons/limboai/icons/" ]; then
    cd ./limboai/demo/addons/limboai/
    ln -s ../../../icons icons || ln -s ../../../limboai/icons icons
    cd -
    highlight -- Linked icons.
else
    highlight -- Skipping linking icons. File already exists!
fi

${PYTHON} limboai/gdextension/update_icons.py --silent
highlight -- Icon declarations updated.

highlight Setup complete.
