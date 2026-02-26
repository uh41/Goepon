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
	StopPlaying();
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

bool EffectBase::StopPlaying()
{
	if(_playHandle == -1)
	{
		return false;
	}
	auto em = EffekseerManager::GetInstance();
	if(!em)
	{
		_playHandle = -1;
		return false;
	}
	// EffekseerManager ‘¤‚Å’âŽ~‚µ‚Ä‚­‚ê‚é
	em->StopEffect(_playHandle);
	_playHandle = -1;
	return true;
}

bool EffectBase::IsPlaying() const
{
	return _playHandle != -1;
}