#include "findeffect.h"
#include "appframe.h"
#include "../AppFrame/source/manager/effekseermanager.h"
#include "enemybase.h"

FindEffect::FindEffect()
{
	Initialize();
}

bool FindEffect::Initialize()
{
	base::Initialize();
	// リソース読み込み（Resources.h の ef::EF_find を想定）
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_find, 1.0f);
	_enemy = nullptr;
	_playHandle = -1;
	return true;
}

bool FindEffect::Terminate()
{
	base::Terminate();
	if(_playHandle != -1)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->StopEffect(_playHandle);
			_playHandle = -1;
		}
	}
	return true;
}

bool FindEffect::Process()
{
	base::Process();
	if(!_enemy) return true;

	auto em = EffekseerManager::GetInstance();
	if(_handle == -1 || !em) return true;

	auto sensor = _enemy->GetEnemySensor();
	bool detected = false;
	if(sensor)
	{
		// 検出／追跡中を検知する条件。必要なら他の条件に差し替え可能。
		detected = sensor->IsChasing();
	}

	// 表示位置：敵の位置 + 腰高さ + オフセット（頭上）
	vec::Vec3 pos = _enemy->GetPos();
	// GetColSubY() は腰位置オフセットのゲッター（存在する想定）
	pos.y += static_cast<float>(_enemy->GetColSubY()) + 150.0f; // +150 は頭上オフセット（調整可）

	if(detected)
	{
		if(_playHandle == -1)
		{
			_playHandle = em->PlayEffect3DPos(_handle, pos);
		}
		else
		{
			em->SetPosEffect(_playHandle, pos);
		}
	}
	else
	{
		if(_playHandle != -1)
		{
			em->StopEffect(_playHandle);
			_playHandle = -1;
		}
	}
	return true;
}

bool FindEffect::Render()
{
	base::Render();
	return true;
}