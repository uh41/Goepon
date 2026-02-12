/*********************************************************************/
// * \file   soundserver.cpp
// * \brief  サウンドサーバー
// *
// * \author ��ؗT�H
// * \date   2025/12/23
// * \��Ɠ�e: �V�K�쐬 ��ؗT�H�@2025/12/23
/*********************************************************************/


#include "soundserver.h"
#include "sounditembase.h"
#include "sounditemoneshot.h"


soundserver::SoundServer::SoundServer()
{
	_bIsUpdate = false;
	_iCntOneShot = 0;
}

soundserver::SoundServer::~SoundServer()
{
	Clear();
}

void soundserver::SoundServer::Clear()
{
	// 保持しているポインタを delete してからクリアする
	for(auto&& e : _v)
	{
		if(e.second)
		{
			delete e.second;
			e.second = nullptr;
		}
	}
	_v.clear();

	for(auto&& e : _vAdd)
	{
		if(e.second)
		{
			delete e.second;
			e.second = nullptr;
		}
	}
	_vAdd.clear();

	for(auto&& e : _vDel)
	{
		if(e.second)
		{
			// _vDel に入っている要素は _v にも存在する可能性があるため
			// 安全のため delete（存在しなければ delete nullptr にはならない）
			delete e.second;
			e.second = nullptr;
		}
	}
	_vDel.clear();
}

void soundserver::SoundServer::Add(SoundItemOneShot* oneshot)
{
	std::string name = "ONESHOT_SOUND" + std::to_string(_iCntOneShot);
	_iCntOneShot++;
	Add(name, at::spc<SoundItemBase>(oneshot));
	oneshot->Play();	// OneShot は即再生
}

void soundserver::SoundServer::Add(std::string name, at::spc<SoundItemBase> sound)
{
	// NOTE:
	// 以前は &sound を格納していたためスタック参照が残り、Get 時に不正になっていました。
	// マップは動的確保した at::spc<SoundItemBase>* を保持する設計になっているため、
	// ここで new して格納します（Del では delete する実装と整合）。
	at::spc<SoundItemBase>* stored = new at::spc<SoundItemBase>(sound);

	if(_bIsUpdate)
	{
		_vAdd[name] = stored;
	}
	else
	{
		_v[name] = stored;
	}
	if(stored->get())
	{
		stored->get()->SetSoundServer(this);
	}
}

bool soundserver::SoundServer::Del(SoundItemBase* sound)
{
	for(auto&& e : _v)
	{
		if(e.second->get() == sound)
		{
			if(_bIsUpdate)
			{
				_vDel[e.first] = e.second;
			}
			else
			{
				std::string name = e.first;
				// _v[name] は at::spc<SoundItemBase>* なので delete してから erase
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
		return Del(_v[name]->get());
	}
	return false;
}

soundserver::SoundItemBase* soundserver::SoundServer::Get(std::string name)
{
	if(_v.count(name) > 0)
	{
		return _v[name]->get();
	}
	return nullptr;
}

void soundserver::SoundServer::StopType(SoundItemBase::TYPE type)
{
	for(auto&& e : _v)
	{
		if(e.second->get()->GetType() == type)
		{
			e.second->get()->Stop();
		}
	}
}

void soundserver::SoundServer::Update()
{
	for(auto&& e : _vAdd)
	{
		_v[e.first] = e.second;
		if(_v[e.first] && _v[e.first]->get())
		{
			_v[e.first]->get()->SetSoundServer(this);
		}
	}
	_vAdd.clear();

	_bIsUpdate = true;
	for(auto&& e : _v)
	{
		if(e.second && e.second->get())
		{
			e.second->get()->Update();
		}
	}
	_bIsUpdate = false;

	for(auto&& e : _vDel)
	{
		Del(e.second->get());
	}
	_vDel.clear();
}