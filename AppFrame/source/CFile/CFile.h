/*********************************************************************/
// * \file   cfile.h
// * \brief  CFileクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once

#include "../container.h"

class CFile
{
public:
	// 読み込み
	CFile(const std::string filename);
	// 書き込み
	CFile(const std::string filename, void* data, int size);		// バイナリデータ
	CFile(const std::string filename, const std::string writestr) 
		: CFile(filename, (void*)writestr.c_str(), (int)writestr.size()) { // 文字列データ
		// 委任コンストラクタで、バイナリデータ書き込みを呼び出す
	}

	~CFile();
	std::string	Filename() { return _fileName; }
	void* Data() { return _data; }
	std::string DataStr() { return std::string(_data); }
	int Size() { return _size; }
	bool Success() { return _success; }

private:
	void Init();

	std::string		_fileName;

	int		_size;
	char* _data;
	bool	_success;
};
