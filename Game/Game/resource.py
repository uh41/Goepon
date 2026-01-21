
import os

HERE = os.path.dirname(os.path.abspath(__file__))
IMG_DIR = os.path.join(HERE, "res")
HEADER_PATH = os.path.join(HERE, "../../AppFrame/source/Resources.h")

def generate_header():
    files = os.listdir(IMG_DIR)
    with open(HEADER_PATH, "w", encoding="utf-8-sig") as f:
        f.write("#pragma once\n\nnamespace img\n{\n")

        for file in files:
            if(file.endswith(".png")):
                name_base = os.path.splitext(file)[0].lower()
                var_name = f"{name_base}"

                full_path = os.path.join(IMG_DIR, file)
                rel_path = os.path.join("res", file).replace("\\", "/")
                f.write(f' inline constexpr const char* {var_name} = "{rel_path}";\n')
        
        f.write("}\n")

        f.write("\nnamespace mv1\n{\n")
        for root, dirs, files in os.walk(IMG_DIR):
            for file in files:
                if(file.endswith(".mv1")):
                    name_base = os.path.splitext(file)[0].lower()
                    var_name = f"{name_base}"

                    full_path = os.path.join(root, file)
                    rel_path = os.path.relpath(full_path, IMG_DIR).replace("\\", "/")
                    f.write(f' inline constexpr const char* {var_name} = "{rel_path}";\n')
        f.write("}\n")

generate_header()