#pragma once
#include "enemybase.h"
#include "appframe.h"
#include "movepointcontroll.h"
#include "applicationglobal.h"

// 前方宣言
class EnemySensor;
class PlayerBase;

class EnemyMove : public EnemyBase
{
	typedef EnemyBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);	// EnemySensorを設定
	void OnPlayerLost();										// プレイヤー見失い時の処理

	// 巡回ルートの設定
	void SetPatrolPoint(const at::vet<vec::Vec3>& point);
	void ProcessPatrol();
	void ProcessReturnToPatrolPoint();

	// 巡回ポイントに到着したときの処理
	void SetPatrolWaitDirection(int id);

	// 巡回中かどうかの判定
	bool IsPatrolling() const {return _isPatrol;}

	// パトロールポイントの情報を設定
	void SetPatrolPointInfo(const at::vec<ApplicationGlobal::PatrolPointInfo>& points);

	void OnDamageStart()override;
	void OnDamageEnd()override;

	virtual float GetHearingRadius() const override { return 300.0f; } // 音検知の半径をオーバーライド
	void StartMoveToSound(const vec::Vec3& soundPos, int soundLevel);

	// 初期巡回位置関連（外部から復元するためのAPI）
	bool HasInitialPatrolIndex() const { return _hasInitialPatrolIndex; }
	int GetInitialPatrolIndex() const { return _initialPatrolIndex; }
	// 初期インデックスへ patrol コントローラと位置を復元する（Reset 後に呼ぶ）
	void RestoreInitialPatrolPosition();

	// CaptureInitialTransform は基底で virtual なので override して初期巡回インデックスをキャプチャする
	virtual void CaptureInitialTransform() override;

protected:
	// 巡回ルート関連
	at::spc<MovePointControll> _patrol;// 巡回ポイント管理クラス
	bool _isPatrol;						// 巡回中かどうか
	float _patrolSpeed;					// 巡回速度
	int _patrolIndex;					// 現在の巡回ポイントのインデックス
	int _savePatrolIndex;				// 戻る前の巡回ポイントのインデックス

	vec::Vec3 _savePoint;				// 戻る前の位置を保存

	// 初期巡回インデックス保存
	int _initialPatrolIndex = 0;
	bool _hasInitialPatrolIndex = false;

	// 初期位置に戻る処理を開始
	void ReturnInitialPos();		

	// テレポート状態のリセット
	void ResetTeleport();

	// 音見地から保存した地点があるか
	bool _hasSavePoint;

	// 巡回ポイント到着後の待機
	bool _bPatrolWaiting;		// 到着後の待機中フラグ
	float _patrolWaitTimer;		// 待機カウントダウン
	float _patrolWaitDuration;	// 待機時間（秒）
	vec::Vec3 _patrolWaitDir;	// 待機中に向く方向

	at::vet<ApplicationGlobal::PatrolPointInfo> _patrolPointInfo;	// パトロールポイントの情報リスト
};

