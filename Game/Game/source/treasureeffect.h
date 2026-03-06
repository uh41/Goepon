#pragma once
#include "effectbase.h"

class TreasureBase;

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

	void SetTreasure(at::vspc<TreasureBase> treasure) { _treasureBase = treasure; }

protected:
	at::vspc<TreasureBase> _treasureBase;

	// •ó” ‚²‚Æ‚ÌÄ¶ƒnƒ“ƒhƒ‹ŠÇ—
	at::umtc<TreasureBase*, int> _playHandles;
};

