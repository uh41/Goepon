/*********************************************************************/
// * \file   soundserver.cpp
// * \brief  サウンドサーバークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 新規作成 鈴木裕稀　2025/12/23
/*********************************************************************/


#include "soundserver.h"
#include "sounditembase.h"

// コンストラクタ
soundserver::SoundServer::SoundServer()
{
	_bIsUpdate = false;
	_iCntOneShot = 0;
}

// デストラクタ
soundserver::SoundServer::~SoundServer()
{
	Clear();
}

// サウンドサーバーのクリア
void soundserver::SoundServer::Clear()
{
	for(auto&& e : _v)
	{
		delete e.second;
	}
	_v.clear();
}

void soundserver::SoundServer::Add(SoundItemOneShot* oneshot)
{
	std::string name = "ONESHOT_SOUND" + std::to_string(_iCntOneShot);
	_iCntOneShot++;
	Add(name, oneshot);
	oneshot->Play();	// OneShotは登録した瞬間再生する
}

void soundserver::SoundServer::Add(std::string name, SoundItemBase* sound)
{
	if(_bIsUpdate)
	{
		_vAdd[name] = sound;
	}
	else
	{
		_v[name] = sound;
	}
	sound->SetSoundServer(this);
}

bool soundserver::SoundServer::Del(SoundItemBase* sound)
{
	for(auto&& e : _v)
	{
		if(e.second == sound)
		{
			if(_bIsUpdate)
			{
				_vDel[e.first] = e.second;
			}
			else
			{
				std::string name = e.first;
				delete _v[name];
				_v.erase(name);
			}
			return true;
		}
	}
	return false;
}

bool soundserver::SoundServer::Del(std::string name)
{
	if(_v.count(name) > 0)
	{
		return Del(_v[name]);
	}
	return false;
}

soundserver::SoundItemBase* soundserver::SoundServer::Get(std::string name)
{
	if(_v.count(name) > 0)
	{
		return _v[name];
	}
	return nullptr;
}

void soundserver::SoundServer::StopType(SoundItemBase::TYPE type)
{
	for(auto&& e : _v)
	{
		if(e.second->GetType() == type)
		{
			e.second->Stop();
		}
	}
}

void soundserver::SoundServer::Update()
{
	// 追加リストのものを追加
	for(auto&& e : _v)
	{
		Add(e.first, e.second); // 追加用コンテナにコピー
	}
	_vAdd.clear();

	// Updateを回す
	_bIsUpdate = true;
	for(auto&& e : _v)
	{
		e.second->Update();
	}
	_bIsUpdate = false;

	// 削除リストのものを削除
	for(auto&& e : _vDel)
	{
		Del(e.second);
	}
	_vDel.clear();

}