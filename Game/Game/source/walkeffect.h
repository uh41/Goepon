/*********************************************************************/
// * \file   walkeffect.h
// * \brief  歩きエフェクトクラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "effectbase.h"
#include "playerbase.h"

class WalkEffect : public EffectBase
{
	typedef EffectBase base;
public:
	WalkEffect();
	virtual ~WalkEffect() = default;
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;

	virtual bool StopPlaying() override;// エフェクト停止

	void SetPlayerPos(PlayerBase* player) { _playerBase = player; }

protected:
	PlayerBase* _playerBase;
	int _stepIntervalFrames;	// 歩くエフェクトの再生間隔
	int _stepCounter;			// 歩くエフェクトのカウント
	at::vet<int> _playHandles;
	bool _wasDash;				// 前フレームのダッシュ状態を記録するフラグ
};