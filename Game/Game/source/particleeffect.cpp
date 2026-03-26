#include "particleeffect.h"
#include "Goal.h"

ParticleEffect::ParticleEffect()
{
	Initialize();
}

bool ParticleEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_Kirakira, 1.0f);
	_playHandle = -1;
	_goal = nullptr;
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
	_goal = nullptr;
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

	if (_playHandle != -1)
	{
		em->SetPosEffect(_playHandle, _goal->GetPos());
	}
}


bool ParticleEffect::Process()
{
	base::Process();

	if (_playHandle != -1)
	{
		auto em = EffekseerManager::GetInstance();
		if (em)
		{
			em->SetPosEffect(_playHandle, _goal->GetPos());
		}
	}

	return true;
}

bool ParticleEffect::Render()
{
	base::Render();
	return true;
}