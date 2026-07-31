/*********************************************************************/
// * \file   findeffect.h
// * \brief  見つけた顔エフェクトクラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

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

	virtual bool StopPlaying() override;// エフェクト停止

	// 敵のリストを設定
	void SetEnemy(at::vspc<EnemyBase> enemy) { _enemy = enemy; }

	void PlayOnce(EnemyBase* enemy); // 敵ごとにエフェクトを再生する

protected:
	at::vspc<EnemyBase> _enemy;
	at::umtc<EnemyBase*, int> _playHandleMap;// 敵ごとのエフェクト
};

