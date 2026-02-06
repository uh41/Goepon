#include "hensineffect.h"

HensinEffect::HensinEffect()
{
	Initialize();
}

HensinEffect::~HensinEffect()
{
	
}

bool HensinEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::hennsin, 1.0f);
	return true;
}

bool HensinEffect::Terminate()
{
	base::Terminate();
	auto em = EffekseerManager::GetInstance();
	if(em && _handle != -1)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}
	return true;
}

void HensinEffect::PlayEffect(const vec::Vec3& pos)
{
	auto em = EffekseerManager::GetInstance();
	if(!em || _handle == -1)
	{
		return;
	}
	em->PlayEffect3DPos(_handle, pos);
}


bool HensinEffect::Process()
{
	base::Process();
	return true;
}

bool HensinEffect::Render()
{
	base::Render();
	return true;
}

