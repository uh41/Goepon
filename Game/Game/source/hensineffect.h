#pragma once
#include "effectbase.h"
class HensinEffect : public EffectBase
{
	typedef EffectBase base;
public:
	HensinEffect();
	virtual ~HensinEffect();
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;

	void PlayEffect(const vec::Vec3& pos);

};

