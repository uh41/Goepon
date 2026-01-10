/*********************************************************************/
// * \file   enemy.h
// * \brief  エネミークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
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

	// EnemySensorとの連携
	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);
	void OnPlayerDetected(const VECTOR& playerPos);
	void OnPlayerLost();

protected:
	// センサー関連
	std::shared_ptr<EnemySensor> _enemySensor;
	bool _detectedPlayer;		// プレイヤーを検出したか
	VECTOR _playerPos;			// 検出したプレイヤーの位置
	float _rotationSpeed;		// 回転速度

	// 移動関連
	float _moveSpeed;			// 移動速度
	VECTOR _targetPosition;		// 目標位置（追跡時の移動先）
	bool _isMoving;				// 移動中かどうか

	// プレイヤーの方向を向く処理
	void LookAtPlayer();
	void UpdateRotationToPlayer();

	// 移動処理
	void MoveTowardsTarget(const VECTOR& target);
	void UpdateChasing();
};