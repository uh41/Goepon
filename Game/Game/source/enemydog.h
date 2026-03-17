#pragma once
#include "enemybase.h"
#include "appframe.h"
#include "movepointcontroll.h"

// 前方宣言
class EnemySensor;
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

	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);

	virtual float GetHearingRadius() const override { return 1000.0f; }

	// 移動範囲を設定
	void SetMovementArea(const std::vector<vec::Vec3>& areaPoints);

private:
	// ランダムウォーク用:_rm
	vec::Vec3 _rmWalkDir;			// ランダム移動の方向
	bool _rmWalking;				// ランダム移動中かどうか
	float _rmWalkTimer;				// ランダム移動のタイマー
	float _rmWalkInterval;			// 次の方向変更までの時間
	float _rmWalkDistance;			// ランダム移動の距離
	float _rmWalkTraveledDistance;	// 移動した距離

	// 方向転換時の待機用:_dc
	bool _isDirectionChange;			// 方向転換待機中フラグ
	float _dcWaitTimer;				// 方向転換待機タイマー
	static constexpr float DC_WAIT_TIME = 1.0f; // 待機時間

	// 移動範囲制限用の変数
	std::vector<vec::Vec3> _movementAreaPoints;	// 移動可能範囲のポリゴン頂点
	bool _MovementArea;							// 移動範囲が設定されているか

	// 音波発生用の変数
	bool _isChasing;			// 前フレームで追跡中だったか
	float _soundEmitTimer;		// 音波発生タイマー
	static constexpr float SOUND_EMIT_INTERVAL = 1.0f; // 音波発生間隔

	// ランダムウォークの処理
	void ProcessRandomWalk();

	// 新しいランダム方向を設定
	void SetNewRandomDirection();

	// 指定した位置が移動範囲内かチェック
	bool IsPosInArea(const vec::Vec3& pos) const;
};

