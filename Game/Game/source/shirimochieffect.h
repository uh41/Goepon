#pragma once
#include "effectbase.h"

class ShirimochiEffect : public EffectBase
{
	typedef EffectBase base;
public:
	ShirimochiEffect();
	virtual ~ShirimochiEffect() = default;
	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

	void PlayEffect(const vec::Vec3& pos) override;
};

