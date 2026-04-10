#pragma once
#include "charabase.h"
#include "appframe.h"
#include "playerbase.h"

class EffectBase : public CharaBase
{
	typedef CharaBase base;
public:
	EffectBase();
	virtual ~EffectBase();

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	virtual void PlayEffect(const vec::Vec3& pos) {}

	virtual bool StopPlaying();

	bool IsPlaying() const;

	void SetTargetPlayer(PlayerBase* player) { _targetPlayer = player; }

	virtual bool PlayeMultiEffect(const at::vet<vec::Vec3>& pos);// 複数の位置でエフェクトを再生する
	virtual bool StopMultiEffect(); // 複数のエフェクトを停止する
	bool IsMultiEffectPlaying() const; // 複数のエフェクトが再生中かどうかを確認する

protected:
	bool _effekseerLaunche;
	int _playHandle;
	at::vet<int> _playHandles; // 複数再生用のハンドルリスト

	PlayerBase* _targetPlayer; // エフェクトの対象となるプレイヤー
};

