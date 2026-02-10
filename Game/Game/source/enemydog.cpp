#include "enemydog.h"
#include "enemysensor.h"
#include "enemysoundsensor.h"

bool EnemyDog::Initialize()
{
	base::Initialize();
	_handle = MV1LoadModel("");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 腰位置の設定
	_fColSubY = 100.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 10.0f;
	_fHp = 30.0f;
	// センサー関連の初期化（共通は EnemyBase::Initialize で行われる）
	_rotationSpeed = 0.5f;                     // 回転速度（調整可能）
	_moveSpeed = 2.0f;
	// 整合性のため他は base に委譲済み
	// タイマー初期化等は base にて行われている
	return true;
}