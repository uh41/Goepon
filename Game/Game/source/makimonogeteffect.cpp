#include "makimonogeteffect.h"

MakimonoGetEffect::MakimonoGetEffect()
{
	Initialize();
}

bool MakimonoGetEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::Howtoplay);
	_playHandle = -1;
	return true;
}

bool MakimonoGetEffect::Terminate()
{
	base::Terminate();
	auto em = EffekseerManager::GetInstance();
	if(em && _playHandle != -1)
	{
		em->StopEffect(_playHandle);
	}
	_playHandle = -1;
	return true;
}

void MakimonoGetEffect::PlayEffect(const vec::Vec3& pos)
{
	auto em = EffekseerManager::GetInstance();
	if(!em || _handle == -1)
	{
		return;
	}

	// 既に再生中なら一度止めて再生し直す（重複を避ける簡易処理）
	if(_playHandle != -1)
	{
		em->StopEffect(_playHandle);
		_playHandle = -1;
	}

	_playHandle = em->PlayEffect3DPos(_handle, pos);
	if(_playHandle != -1 && _targetPlayer)
	{
		em->SetPosEffect(_playHandle, _targetPlayer->GetPos());
	}
}

bool MakimonoGetEffect::Process()
{
	base::Process();

	if(_playHandle != -1 && _targetPlayer)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->SetPosEffect(_playHandle, _targetPlayer->GetPos());
		}
	}

	return true;
}

bool MakimonoGetEffect::Render()
{
	base::Render();
	return true;
}