# -*- coding: utf-8 -*-
import os
import sys

def generate_header():
    try:
        HERE = os.path.dirname(os.path.abspath(__file__))
        IMG_DIR = os.path.join(HERE, "res")
        HEADER_PATH = os.path.join(HERE, "../../AppFrame/source/Resources.h")

        if not os.path.exists(IMG_DIR):
            print(f"Error: {IMG_DIR} not found", file=sys.stderr)
            return

        lines = ["#pragma once\n\nnamespace img\n{\n"]

        if os.path.exists(IMG_DIR):
            for file in os.listdir(IMG_DIR):
                if file.endswith(".png"):
                    name_base = os.path.splitext(file)[0]
                    lines.append(f' inline constexpr const char* {name_base} = "res/{file}";\n')
        
        lines.append("}\n\nnamespace mv1\n{\n")
        
        for root, dirs, files in os.walk(IMG_DIR):
            for file in files:
                if file.endswith(".mv1"):
                    name_base = os.path.splitext(file)[0]
                    full_path = os.path.join(root, file)
                    rel_path = os.path.relpath(full_path, IMG_DIR).replace("\\", "/")
                    lines.append(f' inline constexpr const char* {name_base} = "res/{rel_path}";\n')
        
        lines.append("}\n")
        new_content = "".join(lines)


        if os.path.exists(HEADER_PATH):
            with open(HEADER_PATH, "r", encoding="utf-8-sig") as f:
                if f.read() == new_content:
                    return


        with open(HEADER_PATH, "w", encoding="utf-8-sig") as f:
            f.write(new_content)

    except Exception as e:
        print(f"Python Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    generate_header()