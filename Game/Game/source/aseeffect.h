#pragma once
#include "effectbase.h"
#include "enemybase.h"
#include "playerbase.h"

class AseEffect : public EffectBase
{
	typedef EffectBase base;
public:
	AseEffect();
	virtual ~AseEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void SetPlayer(PlayerBase* player) { _player = player; }
	void SetEnemy(at::vspc<EnemyBase> enemy) { _enemy = enemy; }

protected:
	PlayerBase* _player;
	at::vspc<EnemyBase> _enemy;
};

