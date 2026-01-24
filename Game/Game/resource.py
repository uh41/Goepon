# -*- coding: utf-8 -*-
import os

def generate_header():
    HERE = os.path.dirname(os.path.abspath(__file__))
    IMG_DIR = os.path.join(HERE, "res")
    HEADER_PATH = os.path.normpath(os.path.join(HERE, "../../AppFrame/source/Resources.h"))

    if not os.path.exists(IMG_DIR):
        return

    lines = ["#pragma once\n\n"]

    lines.append("namespace img\n{\n")
    for file in sorted(os.listdir(IMG_DIR)):
        if file.endswith(".png"):
            name_base = os.path.splitext(file)[0]
            lines.append(f'  inline constexpr const char* {name_base} = "res/{file}";\n')
    lines.append("}\n\n")

    lines.append("namespace mv1\n{\n")
    for root, _, files in os.walk(IMG_DIR):
        for file in sorted(files):
            if file.endswith(".mv1"):
                name_base = os.path.splitext(file)[0]
                full_path = os.path.join(root, file)
                rel_path = os.path.relpath(full_path, IMG_DIR).replace("\\", "/")
                lines.append(f'  inline constexpr const char* {name_base} = "res/{rel_path}";\n')
    lines.append("}\n")

    new_content = "".join(lines)

    if os.path.exists(HEADER_PATH):
        with open(HEADER_PATH, "r", encoding="utf-8-sig") as f:
            if f.read() == new_content:
                return

    os.makedirs(os.path.dirname(HEADER_PATH), exist_ok=True)
    with open(HEADER_PATH, "w", encoding="utf-8-sig") as f:
        f.write(new_content)

if __name__ == "__main__":
    generate_header()