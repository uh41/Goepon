#include "particleeffect.h"

ParticleEffect::ParticleEffect()
{
	Initialize();
}

bool ParticleEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_Treasureopen, 1.0f);
	_playHandle = -1;
	_targetPlayer = nullptr;
	return true;
}

bool ParticleEffect::Terminate()
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

void ParticleEffect::PlayEffect(const vec::Vec3& pos)
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


bool ParticleEffect::Process()
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

bool ParticleEffect::Render()
{
	base::Render();
	return true;
}