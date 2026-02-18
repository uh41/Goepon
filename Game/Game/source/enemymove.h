#pragma once
#include "enemybase.h"
#include "appframe.h"
#include "movepointcontroll.h"

// 前方宣言
class EnemySensor;
class PlayerBase;


class EnemyMove : public EnemyBase
{
	typedef EnemyBase base;
public:
	EnemyMove();
	~EnemyMove();
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);	// EnemySensorを設定
	void OnPlayerLost();										// プレイヤー見失い時の処理

	// 敵が初期位置に戻り中かどうかの判定（検知停止用）
	void CaptureInitialTransform();

	// 巡回ルートの設定
	void SetPatrolPoint(const at::vet<vec::Vec3>& point);
	void ProcessPatrol();
	void ProcessReturnToPatrolPoint();

	bool IsPatrolling() const
	{
		return _isPatroll;
	}

	void OnDamageStart()override;
	void OnDamageEnd()override;

	virtual float GetHearingRadius() const override { return 300.0f; } // 音検知の半径をオーバーライド
	void StartMoveToSound(const vec::Vec3& soundPos, int soundLevel);

protected:

	// 巡回ルート関連
	at::spc<MovePointControll> _patroll;// 巡回ポイント管理クラス
	bool _isPatroll;					// 巡回中かどうか
	float _patrolSpeed;					// 巡回速度
	int _patrolIndex;					// 現在の巡回ポイントのインデックス
	int _savePatrolIndex;				// 戻る前の巡回ポイントのインデックス

	vec::Vec3 _savePoint;				// 戻る前の位置を保存

	// 初期位置に戻る処理を開始
	void StartReturningToInitialPosition();		

	// テレポート状態のリセット
	void ResetTeleport();

	// 音見地から保存した地点があるか
	bool _hasSavePoint;

	bool _isPatrolWaiting;		// 到着後の待機中フラグ
	float _patrolWaitTimer;		// 待機カウントダウン
	float _patrolWaitDuration;	// 待機時間（秒）
	vec::Vec3 _patrolWaitDir;	// 待機中に向く方向
};

