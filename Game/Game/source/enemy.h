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
#include "appframe.h"
#include "movepointcontroll.h"

// 前方宣言
class EnemySensor;
class EnemySoundSensor;
class PlayerBase;

class Enemy : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	//void OnPlayerLost();										// プレイヤー見失い時の処理

	// 初期位置と向きをキャプチャ
	void CaptureInitialTransform();

protected:

	// プレイヤーの方向を向く処理
	//void UpdateRotationToPlayer();	// 徐々にプレイヤーの方向を向く

	// 移動処理
	//void MoveTowardsTarget(const vec::Vec3& target);	// 目標位置に向かって移動

	//void StartReturningToInitialPosition();		// 初期位置に戻る処理を開始
	
};
