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

	virtual bool StopPlaying() override;

	void SetPlayerPos(PlayerBase* player) { _playerBase = player; }

protected:
	PlayerBase* _playerBase;

	// 足音等を出す間隔（フレーム）とカウンタ
	int _stepIntervalFrames;
	int _stepCounter;
	at::vet<int> _playHandles;
	bool _wasDash; // 前フレームのダッシュ状態を記録するフラグ
};