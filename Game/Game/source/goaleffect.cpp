#include "goaleffect.h"
#include "Goal.h"

GoalEffect::GoalEffect()
{
	_goalEffectHandle = -1;
}

bool GoalEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::Escapepoint);
	_playHandle = -1;
	_goalEffectHandle = -1;
	return true;
}

bool GoalEffect::Terminate()
{
	base::Terminate();
	if(_goalEffectHandle != -1)
	{
		EffekseerManager::GetInstance()->StopEffect(_goalEffectHandle);
		_goalEffectHandle = -1;
	}
	return true;
}

bool GoalEffect::Process()
{
	base::Process();

	auto em = EffekseerManager::GetInstance();
	if(!em) return true;

	// 前フレームのハンドルを停止してから再生（位置更新のため）
	if(_goalEffectHandle != -1)
	{
		em->StopEffect(_goalEffectHandle);
		_goalEffectHandle = -1;
	}

	if(_goal && _handle != -1)
	{
		int h = em->PlayEffect3DPos(_handle, _goal->GetPos());
		_goalEffectHandle = h;
	}

	return true;
}

bool GoalEffect::Render()
{
	base::Render();
	return true;
}