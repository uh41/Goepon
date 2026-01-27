/*********************************************************************/
// * \file   enemy.h
// * \brief  エネミークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#pragma once
#include "enemybase.h"

// 前方宣言
class EnemySensor;
class PlayerBase;

class Enemy : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);	// EnemySensorを設定
	void OnPlayerDetected(const vec::Vec3& playerPos);			// プレイヤー検出時の処理
	void OnPlayerLost();										// プレイヤー見失い時の処理

	// 敵が初期位置に戻り中かどうかの判定（検知停止用）
	bool IsReturningToInitialPosition() const { return _isReturningToInitialPos; }

	// YouDiedメッセージ表示関連
	void TriggerYouDiedMessage();
	void RenderYouDiedMessage();
	bool IsShowingYouDiedMessage() const { return _showYouDiedMessage; }

	// 初期位置を設定するメソッドを追加
	void SetInitialPosition(const vec::Vec3& position, const vec::Vec3& direction);

public:
	std::shared_ptr<EnemySensor> GetEnemySensor() const { return _enemySensor; }

protected:
	// センサー関連
	std::shared_ptr<EnemySensor> _enemySensor;	// 敵のセンサー
	bool _detectedPlayer;	// プレイヤーを検出したか
	vec::Vec3 _playerPos;	// 検出したプレイヤーの位置
	float _rotationSpeed;	// 回転速度

	// 移動関連
	float _moveSpeed;			// 移動速度
	vec::Vec3 _targetPosition;	// 目標位置（追跡時の移動先）
	bool _isMoving;				// 移動中かどうか

	// プレイヤーの方向を向く処理
	void LookAtPlayer();			// 即座にプレイヤーの方向を向く
	void UpdateRotationToPlayer();	// 徐々にプレイヤーの方向を向く

	// 移動処理
	void MoveTowardsTarget(const vec::Vec3& target);	// 目標位置に向かって移動
	void UpdateChasing();								// 追跡処理

	// 初期位置に戻る機能
	vec::Vec3 _initialPosition;		// 初期位置
	vec::Vec3 _initialDirection;	// 初期向き
	bool _isReturningToInitialPos;	// 初期位置に戻り中かどうか
	float _returnSpeed;				// 初期位置に戻る速度

	// 初期位置に戻る処理
	void UpdateReturningToInitialPosition();	// 初期位置に戻る更新処理
	void StartReturningToInitialPosition();		// 初期位置に戻る処理を開始
	bool IsAtInitialPosition() const;			// 初期位置にいるかどうかをチェック

	// YouDiedメッセージ表示関連
	bool _showYouDiedMessage;
	float _youDiedMessageTimer;
	static constexpr float YOU_DIED_DISPLAY_TIME = 2.0f; // 表示時間（秒）

	// 敵の向き変更タイマー
	float DirChangeTimer;
	static constexpr float DirChangeInterval = 15.0f; // 向き変更の間隔（秒）

	// 床の存在を確認する関数
	bool CheckFloorExistence(const vec::Vec3& position);

	// テレポート状態のリセット
	void ResetTeleport(); 

	// テレポート関連
	bool _waitingForTeleport;		// テレポート待機中フラグ
	float _teleportTimer;			// テレポートまでの待機時間
	static constexpr float TELEPORT_WAIT_TIME = 3.0f; // テレポートまでの待機時間（秒）
};