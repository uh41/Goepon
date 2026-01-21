/*********************************************************************/
// * \file   ResourceServer.cpp
// * \brief  リソース管理クラス
// *
// * \author 石森虹大
// * \date   2026/01/08
// * \作業内容: 新規作成 石森虹大　2026/01/08
/*********************************************************************/

#include "ResourceServer.h"
#include <algorithm>

// 静的メンバ変数の初期化
std::unordered_map<std::string, int> ResourceServer::_mapGraph;
std::unordered_map<std::string, ResourceServer::DIVGRAPH> ResourceServer::_mapDivGraph;
std::unordered_map<std::string, int> ResourceServer::_mapModel;
std::unordered_map<std::string, int> ResourceServer::_mapDivModel;

void ResourceServer::Init()
{
	_mapGraph   .clear();
	_mapDivGraph.clear();
	_mapModel   .clear();
	_mapDivModel.clear();
}

void ResourceServer::Release()
{
	ClearGraph();
}

// グラフィック関連データのクリア
void ResourceServer::ClearGraph()
{
	// すべてのデータの削除をする
	for(auto key = _mapGraph.begin(); key != _mapGraph.end(); key++)
	{
		DeleteGraph(key->second);
	}
	_mapGraph.clear();

	// 分割グラフィックの削除
	for(auto key = _mapDivGraph.begin(); key != _mapDivGraph.end(); key++)
	{
		// 分割グラフィックのハンドルをすべて削除
		for(int i = 0; i < key->second.AllNum; i++)
		{
			DeleteGraph(key->second.handle[i]);
		}
		delete[] key->second.handle;
	}
	_mapDivGraph.clear();

	// モデルデータの削除
	for(auto key = _mapModel.begin(); key != _mapModel.end(); key++)
	{
		DeleteGraph(key->second);
	}
	_mapModel.clear();

	// 分割モデルデータの削除
	for(auto key = _mapDivModel.begin(); key != _mapDivModel.end(); key++)
	{
		DeleteGraph(key->second);
	}
	_mapDivModel.clear();
}

// グラフィック読み込み関数
int ResourceServer::LoadGraph(const TCHAR* FileName)
{
	// キーの検索
	auto key = _mapGraph.find(FileName);
	if(key != _mapGraph.end())
	{
		// キーがあった	
		return key->second;
	}
	// キーがなかった
	int cg = ::LoadGraph(FileName); // DXLIBのAPIを呼ぶので、::を先頭につけ、このクラスの同じ名前の関数と区別する
	// キーとデータをmapに登録
	_mapGraph[FileName] = cg;

	return cg;
}

int ResourceServer::LoadDivGraph(const TCHAR* FileName, int AllNum,
	                             int XNum, int YNum,
	                             int XSize, int YSize, int* HandleBuf)
{
	// キーの検索
	auto key = _mapDivGraph.find(FileName);
	if(key != _mapDivGraph.end())
	{
		// キーがあった
		for (int i = 0; i < AllNum; i++)
		{
			// データをコピー
			HandleBuf[i] = key->second.handle[i];
		}
		return 0;
	}
	// キーがなかった
	int* hbuf = new int[AllNum];
	int err = ::LoadDivGraph(FileName, AllNum, XNum, YNum, XSize, YSize, hbuf);
	if (err == 0)
	{
		//成功
		//キーとデータをmapに登録
		_mapDivGraph[FileName].AllNum = AllNum;
		_mapDivGraph[FileName].handle = hbuf;
		//データをコピー
		for(int i = 0; i < AllNum; i++)
		{
			HandleBuf[i] = hbuf[i];
		}
		return err;
	}
}

int ResourceServer::MV1LoadModel(const TCHAR* FileName)
{
	// キーの検索
	auto key = _mapModel.begin();
	if(key != _mapModel.end())
	{
		//キーがあった
		return MV1DuplicateModel(key->second);
	}
	// キーがなかった
	int model = ::MV1LoadModel(FileName);
	// キーとデータをmapに登録
	_mapModel[FileName] = model;

	return model;
}

int ResourceServer::MV1DeleteModel(int handle)
{
	// 要素の検索
	auto key = _mapModel.begin();

	
	for(; key != _mapModel.end(); ++key)
	{
		if((*key).second == handle)
		{
			break;
		}
	}
	
	if(key != _mapModel.end()) { _mapModel.erase(key); }

	::MV1DeleteModel(handle);
	return 0;
}