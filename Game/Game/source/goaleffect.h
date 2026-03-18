#pragma once
#include "effectbase.h"

class Goal;

class GoalEffect : public EffectBase
{
	typedef EffectBase base;
public:
	GoalEffect();
	virtual ~GoalEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void SetGoal(at::spc<Goal> goal) { _goal = goal; }

protected:
	at::spc<Goal> _goal;
	int _goalEffectHandle; // ゴールごとのエフェクトハンドルを管理するマップ
};

