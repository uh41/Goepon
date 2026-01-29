#include "TextUtil.h"

// 空白チェック
bool TextUtil::IsSpace(unsigned char character)
{
	// 空白・改行・タブかどうかを判定
	return std::isspace(character) != 0 || character == '\r' || character == '\n';
}

// std::stringの前方トリム
void TextUtil::LTrim(std::string& text)
{
	size_t i = 0;
	const size_t n = text.size(); 
	while(i < n && IsSpace(static_cast<unsigned char>(text[i]))) ++i;
	if(i > 0) text.erase(0, i);
}

// std::stringの後方トリム
void TextUtil::RTrim(std::string& text)
{
	// 空かどうかをチェック
	if (text.empty()) return;
	size_t i = text.size();
	// 文字列の後ろから空白文字をスキャンしていく
	while (i > 0 && IsSpace(static_cast<unsigned char>(text[i - 1])))
	{
		// 空白が見つかるたびに、有効な文字数(i)を減らしていく
		--i;
	}
	// 有効な文字以降を削除
	if (i < text.size())
	{
		text.erase(i);
	}
}

// 前後トリム
void TextUtil::Trim(std::string& str)
{
	LTrim(str);
	RTrim(str);
}

// 小文字変換
std::string TextUtil::ToLower(const std::string& input)
{
	std::string dst = input;

	// 1文字ずつ小文字化
	for(size_t i = 0; i < dst.size(); ++i)
	{
		dst[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(dst[i])));
	}
	return dst;
}


// 文字列-> floatの安全バース
bool TextUtil::TryParseFloat(const std::string& text, float& outValue)
{
	// 空チェック
	if (text.empty()) return false;
	// 変換を試みる
	try
	{
		// 文字列をfloatに変換を試みる
		// 末尾に空白や改行(\rなど)が含まれていても、数値部分だけを抽出可能
		outValue = std::stof(text);
		return true;
	}
	catch (...)
	{
		// 数値として解釈できない文字(あいうえお等)の場合、
		// プログラムを落とさずに失敗として処理する
		return false;
	}
}

// 文字列委からkey=valueをバースしてmapを返す(CFilと組み合わせて使う){パーサー処理｝
std::unordered_map<std::string, std::string> TextUtil::ParseKeyValueConfig(const std::string& content)
{
	std::unordered_map<std::string, std::string> result; // 結果格納用マップ
	std::istringstream iss(content);                     // 入力文字列ストリーム
	std::string line;                                    // 行バッファ
	// 1行ずつ取り出す
	while (std::getline(iss, line))
	{
		Trim(line);
		if (line.empty()) continue;
		if (line[0] == '#' || line[0] == ';') continue;

		// '='の位置を検索
		size_t pos = line.find('=');
		// '='が無ければスキップ
		if (pos == std::string::npos) continue;

		// keyとvalを分割
		std::string key = line.substr(0, pos);
		std::string val = line.substr(pos + 1);
		// 前後の空白をトリム
		Trim(key);
		Trim(val);
		// valの後ろの改行コードを削除
		while (!val.empty() && (val.back() == '\r' || val.back() == '\n'))
		{
			val.pop_back();
		}

		// keyかvalが空ならスキップ
		if (key.empty() || val.empty()) continue;

		// 小文字化したkeyで登録
		result[ToLower(key)] = val;
	}
	return result;
}


void TextUtil::staticParseKeyValueConfigFromFile(const std::string filePath, const char* val)
{
	// 設定ファイルから上書き読み込み
	CFile cfgFile("res/Player/player_config.txt");
	if(cfgFile.Success())
	{
		auto config = TextUtil::ParseKeyValueConfig(cfgFile.DataStr());
		// 移動速度
		auto it = config.find(filePath);
		if(it != config.end())
		{
			float val;
			// 変換成功したら上書き
			if(TextUtil::TryParseFloat(it->second, val))
			{
				val = val;
			}
		}
	}
}