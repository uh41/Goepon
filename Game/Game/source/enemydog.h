#pragma once
#include "enemybase.h"
#include "appframe.h"
#include "movepointcontroll.h"

// 前方宣言
class EnemySensor;
class EnemySoundSensor;
class PlayerBase;

class EnemyDog : public EnemyBase
{
	typedef EnemyBase base;
public:
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// 初期位置と向きをキャプチャ
	void CaptureInitialTransform();

	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);

	virtual float GetHearingRadius() const override { return 1000.0f; }

private:
	// ランダムウォーク用の変数
	float _randomWalkTimer;			// ランダム移動のタイマー
	float _randomWalkInterval;		// 次の方向変更までの時間
	vec::Vec3 _randomWalkDir;		// ランダム移動の方向
	bool _isRandomWalking;			// ランダム移動中かどうか
	float _randomWalkDistance;		// ランダム移動の距離
	float _randomWalkTraveledDistance;	// 移動した距離

	// ランダムウォークの処理
	void ProcessRandomWalk();
	// 新しいランダム方向を設定
	void SetNewRandomDirection();
};

