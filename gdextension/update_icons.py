#!/usr/bin/python

import os
import glob

def get_script_dir():
    return os.path.dirname(os.path.realpath(__file__))


def main():
    config_dir = get_script_dir()
    config_path = os.path.join(config_dir, "limboai.gdextension")
    content = ""

    f = open(config_path, "r")
    for line in f:
        if line.startswith("[icons]"):
            break
        content += line
    f.close()

    content += "[icons]\n\n"

    icons_dir = os.path.join(config_dir, "../icons/")
    for icon_file in glob.glob(icons_dir + "/*.svg"):
        icon_file = os.path.basename(icon_file)
        content += os.path.splitext(icon_file)[0] + " = \"res://addons/limboai/icons/" + icon_file + "\"\n"

    f = open(config_path, "w")
    f.write(content)
    f.close()

    print(content)
    print("--------------------------------------------------------------------------------------")
    print("Done!")


if __name__ == "__main__":
    main()
