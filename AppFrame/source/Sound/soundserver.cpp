/*********************************************************************/
// * \file   soundserver.cpp
// * \brief  サウンドサーバークラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#include "soundserver.h"
#include "sounditembase.h"
#include "sounditemoneshot.h"

#ifndef NEW
#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif
#endif

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
   
    at::spc<SoundItemBase>* stored = NEW at::spc<SoundItemBase>(sound);

    // 挿入先マップを決定し、既存のエントリがあれば先に delete してから上書きする
    if(_bIsUpdate)
    {
        auto it = _vAdd.find(name);
        if(it != _vAdd.end())
        {
            delete it->second;
            _vAdd.erase(it);
        }
        _vAdd[name] = stored;
    }
    else
    {
        auto it = _v.find(name);
        if(it != _v.end())
        {
            delete it->second;
            _v.erase(it);
        }
        _v[name] = stored;
    }
    if(stored->get())
    {
        stored->get()->SetSoundServer(this);
    }
}

bool soundserver::SoundServer::Del(SoundItemBase* sound)
{
    // イテレータを使って安全に erase する
    for(auto it = _v.begin(); it != _v.end(); ++it)
    {
        if(it->second && it->second->get() == sound)
        {
            if(_bIsUpdate)
            {
                // 所有権を _vDel に移して _v から erase する（後で一括削除）
                _vDel[it->first] = it->second;
                _v.erase(it);
            }
            else
            {
                std::string name = it->first;
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
    if(_vAdd.count(name) > 0)
    {
        
        delete _vAdd[name];
        _vAdd.erase(name);
        return true;
    }
    return false;
}

soundserver::SoundItemBase* soundserver::SoundServer::Get(std::string name)
{
    if(_v.count(name) > 0)
    {
        return _v[name]->get();
    }
    // 追加中のものも見たい場合は _vAdd をもチェックする
    if(_vAdd.count(name) > 0)
    {
        return _vAdd[name]->get();
    }
    return nullptr;
}

void soundserver::SoundServer::StopType(SoundItemBase::TYPE type)
{
    for(auto&& e : _v)
    {
        if(e.second && e.second->get() && e.second->get()->GetType() == type)
        {
            e.second->get()->Stop();
        }
    }
}

void soundserver::SoundServer::Update()
{
    // _vAdd を _v に移す際、既存エントリがあれば delete して上書きする
    for(auto&& e : _vAdd)
    {
        auto it = _v.find(e.first);
        if(it != _v.end())
        {
            // 既存を削除してから上書き
            delete it->second;
            _v.erase(it);
        }
        _v[e.first] = e.second;
        if(_v[e.first] && _v[e.first]->get())
        {
            _v[e.first]->get()->SetSoundServer(this);
        }
    }
    _vAdd.clear();

	_bIsUpdate = true;// 更新中フラグを立てる
	// _v の各要素の Update() を呼ぶ
    for(auto&& e : _v)
    {
        if(e.second && e.second->get())
        {
            e.second->get()->Update();
        }
    }
    _bIsUpdate = false;

	// _vDel に入っている要素を delete してからクリアする
    for(auto&& e : _vDel)
    {
        if(e.second)
        {
            delete e.second;
            e.second = nullptr;
        }
    }
    _vDel.clear();
}