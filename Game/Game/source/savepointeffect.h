#pragma once
#include "effectbase.h"

class SavePoint;

class SavePointEffect : public EffectBase
{
	typedef EffectBase base;
public:
	SavePointEffect();
	virtual ~SavePointEffect() = default;
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;
	
	void SetSavePoint(at::vspc<SavePoint> sp) { _savePoint = sp; }

protected:

	at::vspc<SavePoint> _savePoint;
	at::umtc<SavePoint*, int> _savePointEffectHandle; // 各セーブポイントに対応するエフェクトのハンドル
};

