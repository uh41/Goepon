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

	// 移動範囲を設定（markerDGRから）
	void SetMovementArea(const std::vector<vec::Vec3>& areaPoints);

private:
	// ランダムウォーク用の変数
	float _randomWalkTimer;			// ランダム移動のタイマー
	float _randomWalkInterval;		// 次の方向変更までの時間
	vec::Vec3 _randomWalkDir;		// ランダム移動の方向
	bool _isRandomWalking;			// ランダム移動中かどうか
	float _randomWalkDistance;		// ランダム移動の距離
	float _randomWalkTraveledDistance;	// 移動した距離

	// 移動範囲制限用の変数
	std::vector<vec::Vec3> _movementAreaPoints;	// 移動可能範囲のポリゴン頂点
	bool _hasMovementArea;						// 移動範囲が設定されているか

	// 音波発生用の変数
	bool _wasChasing;				// 前フレームで追跡中だったか
	float _soundEmitTimer;			// 音波発生タイマー
	static constexpr float SOUND_EMIT_INTERVAL = 1.0f; // 音波発生間隔

	// ランダムウォークの処理
	void ProcessRandomWalk();

	// 新しいランダム方向を設定
	void SetNewRandomDirection();

	// 指定した位置が移動範囲内かチェック
	bool IsPositionInArea(const vec::Vec3& pos) const;
};

