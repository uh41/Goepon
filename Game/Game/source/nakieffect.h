#pragma once
#include "effectbase.h"

class NakiEffect : public EffectBase
{
	typedef EffectBase base;
public:
	NakiEffect();
	~NakiEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void PlayEffect(const vec::Vec3& pos);
	void ResetEffect();

protected:
	bool _isPlay;
};

