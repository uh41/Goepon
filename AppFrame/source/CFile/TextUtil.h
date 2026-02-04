#pragma once
#include"../AppFrame/source/appframe.h"

class TextUtil
{
public:
	static TextUtil* GetInstance()
	{
		static TextUtil instance;
		return &instance;
	}

	// 空白判定
	static bool IsSpace(unsigned char character);

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

	//// ファイルを読みこんで、 key=value をパースして map を返す
	float staticParseKeyValueConfigFromFile(const std::string filePath, const float value);

	// 設定値用の関数
	static bool GetConfig(const at::umss<std::string, std::string>& config, const std::string& key, float& value);	

	// ファイルを1回読み込み、文字列として返す
	static at::umss<std::string, std::string> LoadConfigFile(const std::string& filename);

};

