#pragma once
#include "effectbase.h"

class TreasureopenEffect : public EffectBase
{
	typedef EffectBase base;
public:
	TreasureopenEffect();
	~TreasureopenEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void PlayEffect(const vec::Vec3& pos);


};

