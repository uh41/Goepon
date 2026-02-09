#pragma once
#include "effectbase.h"
#include "enemybase.h"

class HatenaEffect : public EffectBase
{
	typedef EffectBase base;
public:
	HatenaEffect();
	virtual ~HatenaEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void Enemy(at::vspc<EnemyBase>& enemy) { _enemy = enemy; }

	// 1回だけ再生
	void PlayOnce(EnemyBase* enemy);

	// エネミーのリセット
	void ResetEnemyEffect(EnemyBase* enemy);

protected:
	at::vspc<EnemyBase> _enemy;
	at::umtc<EnemyBase*, bool> _playHandleMap;// 敵ごとのエフェクト
};

