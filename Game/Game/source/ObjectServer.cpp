/*********************************************************************/
// * \file   ObjectServer.cpp
// * \brief  オブジェクトサーバークラス
// *
// * \author 石森虹大
// * \date   2026/1/5
// * \作業内容: 新規作成 石森虹大　2026/1/5
/*********************************************************************/


#include"ObjectServer.h"
#include"Player.h"
#include"ModeGame.h"
#include<algorithm>
#include<fstream>

ObjectServer::ObjectServer(ModeGame* game)
:_game(game)
{
	Intialize();
}

ObjectServer::~ObjectServer()
{
	Terminate();
}

bool ObjectServer::Intialize()
{
	return true;
}

bool ObjectServer::Terminate()
{
	//管理しているオブジェクトをすべて削除
	ClearObject();
	return true;
}

bool ObjectServer::Process()
{
	// オブジェクトを巡回処理
	for(auto iter = _objects.begin(); iter != _objects.end(); ++iter)
	{
		if((*iter)->Process())
		{

		}
		else
		{
			return false;
		}
	}
	return true;
}

bool ObjectServer::Render()
{
	// オブジェクトを巡回描画
	for(int i = 0; i < _objects.size(); ++i)
	{
		if(!_objects[i]->Render())
		{
			return false;
		}
	}
	return true;
}

void ObjectServer::AddObject(ObjectBase* obj)
{
	if(obj == nullptr) { return; }

	// 既に本登録されている
	if(std::find(_objects.begin(), _objects.end(), obj) != _objects.end())
	{
		return;
	}

	// 既に追加予約されている
	if(std::find(_addObj.begin(), _addObj.end(), obj) != _addObj.end())
	{
		return;
	}

	// 削除予約されている（＝追加しない）
	if(std::find(_deleteObj.begin(), _deleteObj.end(), obj) != _deleteObj.end())
	{
		return;
	}

	_addObj.emplace_back(obj);
}

void ObjectServer::DeleteObject(ObjectBase* obj)
{
	//既に削除予約されているか
	auto iter = std::find(_deleteObj.begin(), _deleteObj.end(), obj);
	if (iter != _deleteObj.end()) 
	{
		return;
	}

	iter = std::find(_addObj.begin(), _addObj.end(), obj);
	if (iter != _addObj.end()) {
		(*iter)->Terminate();
		delete (*iter);
		_addObj.erase(iter);
		return;
	}
	_deleteObj.emplace_back(obj);
}

bool ObjectServer::ClearObject()
{
	for (auto&& obj : _objects) 
	{
		delete obj;
	}
	_objects.clear();

	for (auto&& obj : _addObj) 
	{
		obj->Terminate();
		delete obj;
	}
	_addObj.clear();

	_deleteObj.clear();
	return true;
}

bool ObjectServer::ProcessInit()
{
	// 巡回処理をする前にオブジェクトの追加と削除をしておく
	for(auto && addObj : _addObj)
	{
		//実際に追加されてから、初期化する
		addObj->Initialize();
		_objects.emplace_back(addObj);
	}
	_addObj.clear();

	for (auto&& deleteObj : _deleteObj)
	{
		auto iter = std::find(_objects.begin(), _objects.end(), deleteObj);
		if (iter != _objects.end()) 
		{
			(*iter)->Terminate();
			delete (*iter);
			_objects.erase(iter);
		}
	}
	_deleteObj.clear();

	return true;
}

bool ObjectServer::LoadDate(std::string stageName)
{
	// マップ読み込み
	_sPath           = "res/map/";
	_sJsonFile       = "marker0127_2.json";
	_sJsonObjectName = "stage";

	// ファイルオープン
	_iFile.open(_sPath + _sJsonFile);
	if(!_iFile) { return false; }

	// JSON読み込み
	nlohmann::json jsonData;
	_iFile >> jsonData;

	// マップオブジェクト生成
	if(_map == nullptr)
	{
		_map = new Map();
		AddObject(_map);
	}

	// JSONデータからステージ情報を取得
	nlohmann::json stage = jsonData.at(_sJsonObjectName);
	// マップモデル読み込み
	for(auto& data : stage)
	{
		// objectName を取得
		const std::string name = data.at("objectName").get<std::string>();

		// 引数 stageName と一致する objectName だけ処理
		if(name != stageName)
		{
			continue;
		}
		// マップクラスにハンドルとJSONデータを設定
		_map->SetJsonDataUE(data);
	}

	return true;
}