#pragma once
#include "effectbase.h"

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

	void SetTreasure(at::vspc<Treasure> treasure) { _treasure = treasure; }

protected:
	at::vspc<Treasure> _treasure;

	// •ó” ‚²‚Æ‚ÌÄ¶ƒnƒ“ƒhƒ‹ŠÇ—
	at::umtc<Treasure*, int> _playHandles;
};

