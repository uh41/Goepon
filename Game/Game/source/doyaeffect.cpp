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
}


bool DoyaEffect::Process()
{
	base::Process();
	return true;
}

bool DoyaEffect::Render()
{
	base::Render();
	return true;
}

