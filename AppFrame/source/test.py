
import os

# ==========================================
# 1. ゲームの設定値（ここを書き換えるだけ！）
# ==========================================
config_data = {
    "SCREEN_WIDTH": 1280,
    "SCREEN_HEIGHT": 720,
    "FADE_FRAME": 60,
    "FADE_WAIT": 60,
    "FADE_OUT_WAIT": 30,
    "PLAYER_SPEED": "speed",
}

# ==========================================
# 2. C++用ヘッダーファイルを書き出す処理
# ==========================================
def generate_header():
    file_name = "gameconfig.h"
    
    # 書き込み開始
    with open(file_name, "w", encoding="utf-8-sig") as f:
        f.write("// このファイルは Python により自動生成されました。直接編集しないでください。\n")
        f.write("#pragma once\n\n")
        
        for key, value in config_data.items():
            # C++の定数（constexpr または #define）として書き出す
            line = f"const constexpr auto {key} = {value};\n"
            f.write(line)
            print(f"定数を生成: {key} = {value}")

    print(f"\n成功: {file_name} を出力しました。")

if __name__ == "__main__":
    generate_header()