#pragma once
#include "effectbase.h"

class StunEffect : public EffectBase
{
	typedef EffectBase base;
public:
	StunEffect();
	virtual ~StunEffect() = default;
	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

	virtual void PlayEffect(const vec::Vec3& pos) override;
	virtual bool StopPlaying() override;
};

