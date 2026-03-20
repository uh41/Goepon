#pragma once
#include "effectbase.h"
#include "playerbase.h"
class MakimonoGetEffect : public EffectBase
{
	typedef EffectBase base;
public:
	MakimonoGetEffect();
	virtual ~MakimonoGetEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	virtual void PlayEffect(const vec::Vec3& pos) override;
};

