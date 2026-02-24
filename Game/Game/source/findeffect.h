#pragma once
#include "effectbase.h"

class EnemyBase;

class FindEffect : public EffectBase
{
	typedef EffectBase base;
public:
	FindEffect();
	virtual ~FindEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	virtual bool StopPlaying() override;

	void SetEnemy(at::vspc<EnemyBase> enemy) { _enemy = enemy; }

protected:
	at::vspc<EnemyBase> _enemy;
	at::umtc<EnemyBase*, int> _playHandleMap;// 敵ごとのエフェクト
};

