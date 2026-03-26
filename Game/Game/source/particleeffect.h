#pragma once
#include "effectbase.h"

class Goal;

class ParticleEffect : public EffectBase
{
	typedef EffectBase base;
public:
	ParticleEffect();
	~ParticleEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void PlayEffect(const vec::Vec3& pos);

	void SetGoal(at::spc<Goal> goal) { _goal = goal; }
protected:
	at::spc<Goal> _goal;
};
