#include "treasureeffect.h"
#include "Treasure.h"

TreasureEffect::TreasureEffect()
{
	Initialize();
}

bool TreasureEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_Tresure, 1.0f);
	_treasure = nullptr;
	_playHandle = -1;

	return true;
}

bool TreasureEffect::Terminate()
{
	base::Terminate();

	auto em = EffekseerManager::GetInstance();
	if(em && _playHandle != -1)
	{
		em->StopEffect(_playHandle);
		_playHandle = -1;
	}


	return true;
}

bool TreasureEffect::Process()
{
	base::Process();

	auto em = EffekseerManager::GetInstance();
	if(!em)
	{
		return true;
	}

	if(_treasure->IsVisible())
	{
		if(_handle != -1 && _playHandle == -1)
		{
			_playHandle = em->PlayEffect3DPos(_handle, _treasure->GetPos());
		}
	}
	else
	{
		if(_playHandle != -1)
		{
			em->StopEffect(_playHandle);
			_playHandle = -1;
		}
	}

	return true;
}

bool TreasureEffect::Render()
{
	base::Render();
	return true;
}
