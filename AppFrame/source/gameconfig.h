// このファイルは Python により自動生成されました。直接編集しないでください。
#pragma once
#include "TextUtil.h"

const constexpr auto SCREEN_WIDTH = 1920;// 1280
const constexpr auto SCREEN_HEIGHT = 1080;//720
const constexpr auto FADE_FRAME = 120;
const constexpr auto FADE_WAIT = 60;
const constexpr auto FADE_OUT_WAIT = 30;

//float speed = GetPlayerSpeed();

constexpr float DEFAULT_PLAYER_SPEED = 5.0f;
// const float PLAYER_SPPED = TextUtil::GetInstance()->staticParseKeyValueConfigFromFile("speed", DEFAULT_PLAYER_SPEED);
inline float GetPlayerSpeed() {
    return static_cast<float>(TextUtil::GetInstance()->staticParseKeyValueConfigFromFile("speed", DEFAULT_PLAYER_SPEED));
}
//float speed = GetPlayerSpeed();