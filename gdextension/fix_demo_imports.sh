#!/bin/bash

## This script fixes icon imports in the demo project.
## It enables scaling and color conversion for SVG icons in the demo project.
##
## Dependencies: bash, sed, find.

# Colors
HIGHLIGHT_COLOR='\033[1;36m' # Light Cyan
NC='\033[0m' # No Color
ERROR_COLOR="\033[0;31m"  # Red

usage() { echo -e "Usage: $0 [--silent]\nRun from limboai root directory!"; }

msg () {  echo -e "$@"; }
highlight() { echo -e "${HIGHLIGHT_COLOR}$@${NC}"; }
error () { echo -e "${ERROR_COLOR}$@${NC}" >&2; }

# Exit if a command returns non-zero status.
set -e

if [ ! -d "${PWD}/demo/" ]; then
    error Aborting: \"demo\" subdirectory is not found.
    msg Tip: Run this script from the limboai root directory.
    msg Command: bash ./gdextension/fix_demo_imports.sh
    exit 1
fi

if test -z "$(find demo/addons/limboai/icons/ -maxdepth 1 -name '*.svg' -print -quit)"; then
    error "No icons found in the demo project!"
    msg Make sure to copy/link the icons into the demo project \(icons/ -\> demo/addons/limboai/icons/\).
    exit 1
fi

if test -z "$(find demo/addons/limboai/icons/ -maxdepth 1 -name '*.import' -print -quit)"; then
    error "No icon import files found!"
    msg Make sure to open the demo project in Godot Editor before running this script.
    exit 1
fi

highlight "--- Listing icons dir:"
ls demo/addons/limboai/icons/
highlight "---"

highlight Applying scale settings...
sed -i 's|editor/scale_with_editor_scale=false|editor/scale_with_editor_scale=true|' demo/addons/limboai/icons/*.import

highlight Applying color conversion settings...
sed -i 's|editor/convert_colors_with_editor_theme=false|editor/convert_colors_with_editor_theme=true|' demo/addons/limboai/icons/*.import

highlight Done!
