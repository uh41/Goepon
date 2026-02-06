#include "effectbase.h"


EffectBase::EffectBase()
{
	Initialize();
}

EffectBase::~EffectBase()
{
	
}

bool EffectBase::Initialize()
{
	base::Initialize();
	EffekseerManager::GetInstance()->Initialize();
	_effekseerLaunche = false;
	_playHandle = -1;

	return true;
}

bool EffectBase::Terminate()
{
	base::Terminate();
	if(_handle != -1)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->DeleteEffect(_handle);
			_handle = -1;
		}
	}

	// Effekseer の終了（Initialize で起動していれば）
	if(_effekseerLaunche)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->Terminate();
			_effekseerLaunche = false;
		}
	}
	return true;
}

bool EffectBase::Process()
{
	base::Process();

	EffekseerManager::GetInstance()->Update();

	return true;
}

bool EffectBase::Render()
{
	base::Render();
	EffekseerManager::GetInstance()->Render();
	return true;
}