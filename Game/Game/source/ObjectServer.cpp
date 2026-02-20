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
#include "Map1.h"
#include "applicationglobal.h"

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
	//�Ǘ����Ă���I�u�W�F�N�g����ׂč폜
	ClearObject();
	return true;
}

bool ObjectServer::Process()
{
	// �I�u�W�F�N�g����񏈗�
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
	// �I�u�W�F�N�g�����`��
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

	// ���ɖ{�o�^����Ă���
	if(std::find(_objects.begin(), _objects.end(), obj) != _objects.end())
	{
		return;
	}

	// ���ɒǉ��\�񂳂�Ă���
	if(std::find(_addObj.begin(), _addObj.end(), obj) != _addObj.end())
	{
		return;
	}

	// �폜�\�񂳂�Ă���i���ǉ����Ȃ��j
	if(std::find(_deleteObj.begin(), _deleteObj.end(), obj) != _deleteObj.end())
	{
		return;
	}

	_addObj.emplace_back(obj);
}

void ObjectServer::DeleteObject(ObjectBase* obj)
{
	//���ɍ폜�\�񂳂�Ă��邩
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
		// Mapもここで delete される可能性があるので、先に監視
		if (obj == _map)
		{
			_map = nullptr;
		}
		delete obj;
	}
	_objects.clear();

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
	// ���񏈗������O�ɃI�u�W�F�N�g�̒ǉ��ƍ폜����Ă���
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

bool ObjectServer::LoadDate(std::string stageName)
{
	const ApplicationGlobal::StageData* stageData = gGlobal.GetStageData(stageName);
	if(stageData != nullptr)
	{
		// マップオブジェクト生成（未生成なら）
		if(_map == nullptr)
		{
			_map = NEW Map1();
			AddObject(_map);
		}

		// StageData 内のオブジェクトリストから指定のオブジェクト名を探す
		for(const auto& objData : stageData->object)
		{
			// StageObjectData::objectName と一致するものを探す
			if(objData.objectName == stageName)
			{
				_map->SetJsonDataUE(objData.json);
				return true;
			}
		}

		// 見つからなかった場合はフォールバックとして最初のオブジェクトを使う（安全策）
		if(!stageData->object.empty())
		{
			_map->SetJsonDataUE(stageData->object.front().json);
			return true;
		}

		// stageData が空なら失敗
		return false;
	}

	return true;

}