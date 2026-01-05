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
#include<unordered_map>

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
	// 既に追加されているか
	auto iter = std::find(_deleteObj.begin(), _deleteObj.end(), obj);
	if(iter != _deleteObj.end())
	{
		return;
	}
	// まだ、追加されていないだけで予約は入っているのでは？
	iter = std::find(_addObj.begin(), _addObj.end(), obj);
	if(iter != _addObj.end())
	{
		return;
	}

	// 初期化
	obj->Initialize();
	_addObj.emplace_back(obj);
}

void ObjectServer::DeleteObject(ObjectBase* obj)
{
	//既に削除予約されているか
	auto iter = std::find(_deleteObj.begin(), _deleteObj.end(), obj);
	if (iter != _deleteObj.end()) {
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
	std::string stage = stageName.substr(0, 1);
	std::string area = stageName.substr(2, 1);

	//ファイルのパス
	std::string filePath = "res/stage/stage" + stage + "/" + area + "/";

	//ファイルの読み込み
	std::ifstream layoutFile(filePath + "Layout.json");

	//Json
	nlohmann::json layoutJson;

	//シリアライズ
	if (!layoutFile) { return false; }
	layoutFile >> layoutJson;

	//プレイヤーの読み込み
	for (auto&& object : layoutJson.at("player"))
	{
		std::string name = object.at("objectName");
		if(name == "marker1")
		{
			Player* player = new Player();
			player->SetJsonDataUE(object);
		}
	}
	
}