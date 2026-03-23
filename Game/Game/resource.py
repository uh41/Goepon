# -*- coding: utf-8 -*-
import os

def generate_header():
    HERE = os.path.dirname(os.path.abspath(__file__))
    IMG_DIR = os.path.join(HERE, "res")
    HEADER_PATH = os.path.normpath(os.path.join(HERE, "../../AppFrame/source/Resources.h"))

    if not os.path.exists(IMG_DIR):
        return

    ns_dict = {
        "img": [],
        "texture": [],
        "ui": [],
        "Title": [],
        "Prologue": [],
        "ef": [],
        "mv1": [],
        "mp3": [],
        "mp4": [],
        "wav": []
    }

    for root, _, files in os.walk(IMG_DIR):
        rel_dir = os.path.relpath(root, IMG_DIR).lower().replace("\\", "/")
        
        for file in sorted(files):
            ext = os.path.splitext(file)[1].lower()
            name_base = os.path.splitext(file)[0]
            
            full_rel_path = os.path.relpath(os.path.join(root, file), IMG_DIR).replace("\\", "/")
            line = f'  inline constexpr const char* {name_base} = "res/{full_rel_path}";\n'

            if ext == ".png":
                if rel_dir == ".":
                    ns_dict["img"].append(line)
                elif "texture" in rel_dir:
                    ns_dict["texture"].append(line)
                elif "ui" in rel_dir:
                    ns_dict["ui"].append(line)
                elif "Title" in rel_dir:
                    ns_dict["Title"].append(line)
                elif "Prologue" in rel_dir:
                    ns_dict["Prologue"].append(line)
            
            elif ext == ".efkefc":
                ns_dict["ef"].append(line)
            
            elif ext == ".mv1":
                ns_dict["mv1"].append(line)
                
            elif ext == ".mp3":
                ns_dict["mp3"].append(line)

            elif ext == ".mp4":
                ns_dict["mp4"].append(line)
                
            elif ext == ".wav":
                ns_dict["wav"].append(line)

    lines = ["#pragma once\n\n"]
    for ns_name, content in ns_dict.items():
        if content:
            lines.append(f"namespace {ns_name}\n{{\n")
            lines.extend(content)
            lines.append("}\n\n")

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