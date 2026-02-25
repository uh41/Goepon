#include "doyaeffect.h"

DoyaEffect::DoyaEffect()
{
	Initialize();
}

bool DoyaEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_doya, 1.0f);
	_playHandle = -1;
	_targetPlayer = nullptr;
	return true;
}

bool DoyaEffect::Terminate()
{
	base::Terminate();

	auto em = EffekseerManager::GetInstance();
	if(em && _handle != -1)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}

	StopPlaying();
	_targetPlayer = nullptr;
	return true;
}

void DoyaEffect::PlayEffect(const vec::Vec3& pos)
{
	auto em = EffekseerManager::GetInstance();
	if(!em || _handle == -1)
	{
		return;
	}
	_playHandle = em->PlayEffect3DPos(_handle, pos);

	if(_playHandle != -1 && _targetPlayer)
	{
		em->SetPosEffect(_playHandle, _targetPlayer->GetPos());
	}
}


bool DoyaEffect::Process()
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

bool DoyaEffect::Render()
{
	base::Render();
	return true;
}

