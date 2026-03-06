#include "stuneffect.h"

StunEffect::StunEffect()
{
	Initialize();
}

bool StunEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_Kizetsu);
	_playHandle = -1;
	return true;
}

bool StunEffect::Terminate()
{
	base::Terminate();

	// エフェクトのリソースを解放
	auto em = EffekseerManager::GetInstance();
	if(em && _handle != -1)
	{
		em->DeleteEffect(_handle);   // エフェクトのリソースを解放
		_handle = -1;
		_playHandle = -1;
	}

	StopPlaying(); // 再生中のエフェクトを停止
	return true;
}

void StunEffect::PlayEffect(const vec::Vec3& pos)
{
	if(_handle == -1)
	{
		return;
	}

	auto em = EffekseerManager::GetInstance();
	if(!em)
	{
		return;
	}

	// 再生中なら「再生し直さず」位置だけ追従させる
	if(_playHandle != -1)
	{
		em->SetPosEffect(_playHandle, pos);
		return;
	}

	// 未再生なら開始
	_playHandle = em->PlayEffect3DPos(_handle, pos);
}

bool StunEffect::StopPlaying()
{
	if(_playHandle != -1)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->StopEffect(_playHandle);
			_playHandle = -1;
			return true;
		}
	}
	return false;
}

bool StunEffect::Process()
{
	base::Process();
	return true;
}

bool StunEffect::Render()
{
	base::Render();
	return true;
}