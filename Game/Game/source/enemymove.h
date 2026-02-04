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

	void CaptureInitialTransform();

	// 巡回ルートの設定
	void SetPatrolPoint(const at::vet<vec::Vec3>& point);
	void ProcessPatrol();
	void ProcessReturnToPatrolPoint();

	bool IsPatrolling() const {
		return _isPatroll;
	}

protected:

	// 巡回ルート関連
	at::spc<MovePointControll> _patroll;// 巡回ポイント管理クラス
	bool _isPatroll;					// 巡回中かどうか
	float _patrolSpeed;					// 巡回速度
	int _patrolIndex;				// 現在の巡回ポイントのインデックス
	int _savePatrolIndex;			// 戻る前の巡回ポイントのインデックス

	vec::Vec3 _savePoint;			// 戻る前の位置を保存

	void StartReturningToInitialPosition();		// 初期位置に戻る処理を開始

	// テレポート状態のリセット
	void ResetTeleport();

};

