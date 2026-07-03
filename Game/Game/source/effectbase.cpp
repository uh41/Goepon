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
	_playHandles.clear();
	return true;
}

bool EffectBase::Terminate()
{
	base::Terminate();
	StopPlaying();
	StopMultiEffect();
	return true;
}

bool EffectBase::Process()
{
	base::Process();

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

bool EffectBase::PlayeMultiEffect(const at::vet<vec::Vec3>& pos)
{
	for(auto& pos : pos)
	{
		PlayEffect(pos);
	}
	return true;
}

bool EffectBase::StopMultiEffect()
{
	if(_playHandles.empty())
	{
		return false;
	}

	auto em = EffekseerManager::GetInstance();
	if(!em)
	{
		_playHandles.clear();
		return false;
	}
	for(auto& handle : _playHandles)
	{
		if(handle != -1)
		{
			em->StopEffect(handle);
		}
	}
	_playHandles.clear();
	return true;
}

bool EffectBase::IsPlaying() const
{
	return _playHandle != -1;
}

bool EffectBase::IsMultiEffectPlaying() const
{
	return !_playHandles.empty();
}