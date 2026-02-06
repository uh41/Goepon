#pragma once
#include "effectbase.h"
#include "appframe.h"

class Treasure;

class TreasureEffect : public EffectBase
{
	typedef EffectBase base;
public:
	TreasureEffect();
	virtual ~TreasureEffect() = default;

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void SetTreasure(Treasure* treasure) { _treasure = treasure; }

protected:
	Treasure* _treasure;

};

