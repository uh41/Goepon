#pragma once
#include "effectbase.h"
#include "appframe.h"


class WalkEffect : public EffectBase
{
	typedef EffectBase base;
public:
	WalkEffect();
	virtual ~WalkEffect();
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;

	void SetPlayerPos(CharaBase* player) { _chara = player; }

protected:
	CharaBase* _chara;

};

