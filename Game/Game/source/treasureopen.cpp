/*********************************************************************/
// * \file   treasureopen.cpp
// * \brief  宝箱が開くエフェクトクラス
// *
// * \author 鈴木裕稀
/*********************************************************************/


#include "treasureopen.h"

TreasureopenEffect::TreasureopenEffect()
{
	Initialize();
}

bool TreasureopenEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_Treasureopen, 1.0f);
	_playHandle = -1;
	_targetPlayer = nullptr;
	return true;
}

bool TreasureopenEffect::Terminate()
{
	base::Terminate();

	auto em = EffekseerManager::GetInstance();
	if (em && _handle != -1)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}

	StopPlaying();
	_targetPlayer = nullptr;
	return true;
}

void TreasureopenEffect::PlayEffect(const vec::Vec3& pos)
{
	auto em = EffekseerManager::GetInstance();
	if (!em || _handle == -1)
	{
		return;
	}
	_playHandle = em->PlayEffect3DPos(_handle, pos);

	if (_playHandle != -1 && _targetPlayer)
	{
		em->SetPosEffect(_playHandle, _targetPlayer->GetPos());
	}
}


bool TreasureopenEffect::Process()
{
	base::Process();

	if (_playHandle != -1 && _targetPlayer)
	{
		auto em = EffekseerManager::GetInstance();
		if (em)
		{
			em->SetPosEffect(_playHandle, _targetPlayer->GetPos());
		}
	}

	return true;
}

bool TreasureopenEffect::Render()
{
	base::Render();
	return true;
}