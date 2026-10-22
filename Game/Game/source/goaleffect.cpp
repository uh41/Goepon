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
    if(!em || !_goal || _handle == -1)
    {
        return true;
    }

    if(!_goal->IsCollisionEnabled())
    {
        if(_goalEffectHandle != -1)
        {
            em->StopEffect(_goalEffectHandle);
            _goalEffectHandle = -1;
        }
        return true;
    }

    // エフェクトが未再生、または再生終了したらゴール位置で再生する
    if(_goalEffectHandle == -1 || !em->IsPlayingEffect(_goalEffectHandle))
    {
        _goalEffectHandle = em->PlayEffect3DPos(_handle, _goal->GetPos());
    }


	return true;
}

bool GoalEffect::Render()
{
	base::Render();
	return true;
}