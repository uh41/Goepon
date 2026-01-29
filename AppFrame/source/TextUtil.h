#pragma once
#include"../AppFrame/source/appframe.h"

class TextUtil
{
public:
	// 空白判定
	static bool IsSpace(unsigned char character);

	static TextUtil* GetInstance()
	{
		static TextUtil instance;
		return &instance;
	}

	// トリム
	static void LTrim(std::string& text);
	static void RTrim(std::string& text);
	static void Trim (std::string& text);

	// 小文字変換
	static std::string ToLower(const std::string& input);

	// 文字列-> floatの安全変換
	static bool TryParseFloat(const std::string& text, float& outValue);

	// 文字列からkey=value をパースして map を返す
	static std::unordered_map<std::string, std::string> ParseKeyValueConfig(const std::string& content);

	// ファイルを読みこんで、 key=value をパースして map を返す
	float staticParseKeyValueConfigFromFile(const std::string filePath, const float value);
};

