#include "hatenaeffect.h"

HatenaEffect::HatenaEffect()
{
	Initialize();
}

bool HatenaEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_hatena, 1.0f);
	_enemy = at::vspc<EnemyBase>();
	_playHandleMap.clear();
	return true;
}

bool HatenaEffect::Terminate()
{
	base::Terminate();

	auto em = EffekseerManager::GetInstance();
	if(em && _handle != -1)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}
	StopPlaying();
	return true;
}

void HatenaEffect::PlayOnce(EnemyBase* enemy)
{
	if(!enemy)
	{
		return;
	}
	if(_handle == -1)
	{
		return;
	}

	// すでに再生中なら何もしない
	if(_playHandleMap.find(enemy) != _playHandleMap.end() && _playHandleMap[enemy])
	{
		return;
	}

	EffekseerManager::GetInstance()->PlayEffect3DPos(_handle, enemy->GetPos());

	// 再生中フラグを立てる
	_playHandleMap[enemy] = true;
}

void HatenaEffect::ResetEnemyEffect(EnemyBase* enemy)
{
	if(!enemy)
	{
		return;
	}
	// 再生中フラグをクリアする
	_playHandleMap[enemy] = false;
}

bool HatenaEffect::Process()
{
	base::Process();
	return true;
}

bool HatenaEffect::Render()
{
	base::Render();
	return true;
}

