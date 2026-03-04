/*********************************************************************/
// * \file   ObjectServer.cpp
// * \brief  �I�u�W�F�N�g�T�[�o�[�N���X
// *
// * \author �ΐX����
// * \date   2026/1/5
// * \��Ɠ�e: �V�K�쐬 �ΐX����@2026/1/5
/*********************************************************************/

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif


#include"ObjectServer.h"
#include"Player.h"
#include"ModeGame.h"
#include<algorithm>
#include<fstream>
#include "MapFactory.h"
#include "applicationglobal.h"

// コンストラクタ・デストラクタ
ObjectServer::ObjectServer(ModeGame* game)
	: _player(nullptr)
	, _game(game)
	, _map(nullptr) 
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
	ClearObject();
	return true;
}

bool ObjectServer::Process()
{
	for(auto* obj : _objects)
	{
		if(!obj->Process())
		{
			return false;
		}
	}
	return true;
}

bool ObjectServer::Render()
{
	for(auto* obj : _objects)
	{
		if(!obj->Render())
		{
			return false;
		}
	}
	return true;
}

// オブジェクトの追加
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

	// 削除予約されている（追加しない）
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
	if(iter != _deleteObj.end())
	{
		return;
	}

	iter = std::find(_addObj.begin(), _addObj.end(), obj);
	if(iter != _addObj.end()) {
		(*iter)->Terminate();
		delete (*iter);
		_addObj.erase(iter);
		return;
	}
	_deleteObj.emplace_back(obj);
}

bool ObjectServer::ClearObject()
{
	// まずポインタを退避してからコンテナを空にする（破棄中の再入に強くする）
	std::vector<ObjectBase*> objects = std::move(_objects);
	std::vector<ObjectBase*> addObj = std::move(_addObj);
	std::vector<ObjectBase*> delObj = std::move(_deleteObj);

	_objects.clear();
	_addObj.clear();
	_deleteObj.clear();

	// _map が objects/addObj/delObj のどれで消されてもよいように監視
	for(auto* obj : objects)
	{
		if(obj == _map)
		{
			_map = nullptr;
		}
		delete obj;
	}

	for(auto* obj : addObj)
	{
		if(!obj) { continue; }
		obj->Terminate();
		if(obj == _map)
		{
			_map = nullptr;
		}
		delete obj;
	}

	// DeleteObject() で _deleteObj に積んだものが残って終了するケースを想定して破棄
	for(auto* obj : delObj)
	{
		if(!obj) { continue; }
		obj->Terminate();
		if(obj == _map)
		{
			_map = nullptr;
		}
		delete obj;
	}

	return true;
}

bool ObjectServer::ProcessInit()
{
	// 追加されたオブジェクトを初期化してから _objects に移す
	for(auto && addObj : _addObj)
	{
		//���ۂɒǉ�����Ă���A����������
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

bool ObjectServer::LoadDate(const std::string stageName)
{
	const ApplicationGlobal::StageData* stageData = gGlobal.GetStageData(stageName);
	if(stageData == nullptr)
	{
		return false;
	}

	std::string mapTypeName = "Map1";
	if(stageName == "Stage1")
	{
		mapTypeName = "Map1";
	}
	else if(stageName == "Stage2")
	{
		mapTypeName = "Map2";
	}
	else if(stageName == "Stage3")
	{
		mapTypeName = "Map3";
	}

	// 既存マップがあり、IDが違うなら作り直す
	if(_map != nullptr && _map->GetMapId() != mapTypeName)
	{
		DeleteObject(_map);
		ProcessInit();
		_map = nullptr;
	}

	// マップがないなら作る
	if(_map == nullptr)
	{
		MapType mapType = MapFactory::GetMapTypeFromString(mapTypeName);
		_map = MapFactory::CreateMap(mapType); 
		if(_map == nullptr)
		{
			return false;
		}

		_map->SetMapId(mapTypeName);
		AddObject(_map);
	}
	else
	{
		// 同じマップを継続利用する場合もIDは保証
		_map->SetMapId(mapTypeName);
	}

	return true;
}
