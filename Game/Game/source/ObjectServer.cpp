/*********************************************************************/
// * \file   ObjectServer.cpp
// * \brief  �I�u�W�F�N�g�T�[�o�[�N���X
// *
// * \author �ΐX����
// * \date   2026/1/5
// * \��Ɠ�e: �V�K�쐬 �ΐX����@2026/1/5
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
	// �}�b�v�ǂݍ���
	_sPath           = "res/map/";
	_sJsonFile       = "marker0127_2.json";
	_sJsonObjectName = "stage";

	// �t�@�C���I�[�v��
	_iFile.open(_sPath + _sJsonFile);
	if(!_iFile) { return false; }

	// JSON�ǂݍ���
	nlohmann::json jsonData;
	_iFile >> jsonData;

	// �}�b�v�I�u�W�F�N�g����
	if(_map == nullptr)
	{
		_map = new Map();
		AddObject(_map);
	}

	// JSON�f�[�^����X�e�[�W����擾
	nlohmann::json stage = jsonData.at(_sJsonObjectName);
	// �}�b�v���f���ǂݍ���
	for(auto& data : stage)
	{
		// objectName ��擾
		const std::string name = data.at("objectName").get<std::string>();

		// ���� stageName �ƈ�v���� objectName ��������
		if(name != stageName)
		{
			continue;
		}
		// �}�b�v�N���X�Ƀn���h����JSON�f�[�^��ݒ�
		_map->SetJsonDataUE(data);
		// ���f���ǂݍ���
		const std::string filename = _sPath + name + ".mv1";
		/*const int handle = */
	}

	return true;
}