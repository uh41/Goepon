#pragma once
#include "effectbase.h"

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


};
