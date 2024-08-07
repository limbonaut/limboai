#!/bin/bash

## This script sets up limboai project for development with GDExtension.
## It adds missing files to the demo project for development with GDExtension, and downloads godot-cpp.
## Tested only on Unix-likes. You can perform similar steps manually, if you are on Windows. Check Overview below.
##
## Instructions:
## 1) Clone the limboai repository:
##    git clone https://github.com/limbonaut/limboai
## 2) From the limboai root directory, run:
##    bash ./gdextension/setup_gdextension.sh
##
## Overview:
##   limboai/ -- LimboAI repository after you clone it - call this script from here.
##   limboai/godot-cpp/ -- git repo that will be cloned by this script, unless already exists.
##   limboai/demo/addons/limboai/limboai.gdextension -- symbolic link created (leads to limboai/gdextension/limboai.gdextension).
##   limboai/demo/addons/limboai/icons/ -- symbolic link created (leads to limboai/icons/).
##
##   Note: Script creates symbolic links unless --copy-all is set, in which case it copies the files.
##
## Dependencies: bash, git, python3, trash (optional).

# Script Settings
GODOT_CPP_VERSION=4.2
PYTHON=python

# Colors
HIGHLIGHT_COLOR='\033[1;36m' # Light Cyan
NC='\033[0m' # No Color
ERROR_COLOR="\033[0;31m"  # Red

usage() { echo "Usage: $0 [--copy-all] [--trash-old]"; }

msg () {  echo -e "$@"; }
highlight() { echo -e "${HIGHLIGHT_COLOR}$@${NC}"; }
error () { echo -e "${ERROR_COLOR}$@${NC}" >&2; }

if [ ! -d "${PWD}/demo/" ]; then
    error Aborting: \"demo\" subdirectory is not found.
    msg Tip: Run this script from the limboai root directory.
    msg Command: bash ./gdextension/setup_gdextension.sh
    exit 1
fi

# Interrupt execution and exit on Ctrl-C
trap exit SIGINT

set -e

copy_all=0
trash_old=0

# Parsing arguments
for i in "$@"
do
    case "${i}" in
	--copy-all)
	    copy_demo=1
	    copy_all=1
	    shift
	    ;;
    --trash-old)
        trash_old=1
        shift
        ;;
	*)
	    usage
        exit 1
	    ;;
    esac
done

highlight Setup started.

${PYTHON} gdextension/update_icons.py --silent
highlight -- Icon declarations updated.

transfer="ln -s"
transfer_word="Linked"
if [ ${copy_all} == 1 ]; then
    transfer="cp -R"
    transfer_word="Copied"
fi

if [ ${trash_old} == 1 ]; then
    if ! command -v trash &> /dev/null; then
        error trash command not available. Aborting.
        exit 1
    fi
    trash demo/addons/limboai || /bin/true
    highlight -- Trashed old setup.
fi

if [ ! -d "${PWD}/godot-cpp/" ]; then
    highlight -- Cloning godot-cpp...
    git clone -b ${GODOT_CPP_VERSION} https://github.com/godotengine/godot-cpp
    highlight -- Finished cloning godot-cpp.
else
    highlight -- Skipping \"godot-cpp\". Directory already exists!
fi

if [ ! -e "${PWD}/demo/addons/limboai/bin/limboai.gdextension" ]; then
    mkdir -p ./demo/addons/limboai/bin/
    cd ./demo/addons/limboai/bin/
    ${transfer} ../../../../gdextension/limboai.gdextension limboai.gdextension
    cd -
    highlight -- ${transfer_word} limboai.gdextension.
else
    highlight -- Skipping limboai.gdextension. File already exists!
fi

if [ ! -e "${PWD}/demo/addons/limboai/icons/" ]; then
    cd ./demo/addons/limboai/
    ${transfer} ../../../icons icons
    cd -
    highlight -- ${transfer_word} icons.
else
    highlight -- Skipping icons. File already exists!
fi

highlight Setup complete.
