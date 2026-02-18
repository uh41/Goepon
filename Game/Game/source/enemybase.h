/*********************************************************************/
// * \file   enemybase.h
// * \brief  エネミーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//              センサーが追跡中関数の追加 石森虹大　2026/02/12
/*********************************************************************/

#pragma once
#include "charabase.h"
#include "appframe.h"
#include "movepointcontroll.h"
#include "effectbase.h"

// 前方宣言
class EnemySensor;
class EnemySoundSensor;
class PlayerBase;
class EnemySoundManager;

class EnemyBase : public CharaBase
{
	typedef CharaBase base;

public:

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);	// EnemySensorを設定
	void SetEnemySoundSensor(std::shared_ptr<EnemySoundSensor> sensor);
	void OnPlayerDetected(const vec::Vec3& playerPos);			// プレイヤー検出時の処理
	void OnPlayerLost();										// プレイヤー見失い時の処理

	// 敵が初期位置に戻り中かどうかの判定（検知停止用）
	bool IsReturningToInitialPosition() const { return _isReturningToInitialPos; }

	// 初期位置に戻る処理
	void UpdateReturningToInitialPosition();	// 初期位置に戻る更新処理
	void StartReturningToInitialPosition();		// 初期位置に戻る処理を開始
	bool IsAtInitialPosition() const;			// 初期位置にいるかどうかをチェック

	// プレイヤーの方向を向く処理
	void LookAtPlayer();			// 即座にプレイヤーの方向を向く
	void UpdateRotationToPlayer();	// 徐々にプレイヤーの方向を向く

	// 移動処理
	void MoveTowardsTarget(const vec::Vec3& target);	// 目標位置に向かって移動
	void UpdateChasing();								// 追跡処理

	// 床の存在を確認する関数
	bool CheckFloorExistence(const vec::Vec3& position);

	// テレポート状態のリセット
	void ResetTeleport();

	// ダメージ受けたときの処理
	void StartDamage();
	// 無敵中かどうか
	bool GetIsInvincible() const { return _isInvincible; }

	// デバッグ用：YouDiedメッセージ表示関連
	void TriggerYouDiedMessage();
	void RenderYouDiedMessage();
	bool IsShowingYouDiedMessage() const { return _showYouDiedMessage; }
	void RenderDamageTime();

	bool IsDetectPlayer() const { return _detectedPlayer; }

	vec::Vec3 GetInitialPosition() const { return _initialPosition; }
	vec::Vec3 GetInitialDirection() const { return _initialDirection; }

	void CaptureInitialTransform();

	std::shared_ptr<EnemySensor> GetEnemySensor() const { return _enemySensor; }
	std::shared_ptr<EnemySoundSensor> GetEnemySoundSensor() const { return _enemySoundSensor; }
	std::shared_ptr<EnemySoundSensor> GetSoundSensor() const { return _enemySoundSensor; }

	void SetEffect(at::spc<EffectBase> effect) { _effect = effect; }

	// センサーが追跡中かどうか
	bool IsPlayerChasing() const; 
	void UpdateDamageAnimation();// ダメージアニメーションの更新

	virtual void OnDamageStart() {} // ダメージアニメーション開始時の処理
	virtual void OnDamageEnd() {}   // ダメージアニメーション終了時の処理

	bool IsStun() const { return _isInvincible && _attachStage == 2; }

	void SetEnemyId(uint32_t id) { _enemyId = id; }
	uint32_t GetEnemyId() const { return _enemyId; }

	virtual float GetHearingRadius() const { return 0.0f; } // 音検知の半径
	bool IsMovingToSound() const { return _isMovingToSound; }

protected:
	uint32_t _enemyId; // 敵のID

	// センサー関連
	std::shared_ptr<EnemySensor> _enemySensor;	// 敵のセンサー
	std::shared_ptr<EnemySoundSensor> _enemySoundSensor;

	bool _detectedPlayer;	// プレイヤーを検出したか
	vec::Vec3 _playerPos;	// 検出したプレイヤーの位置
	float _rotationSpeed;	// 回転速度

	// 移動関連
	float _moveSpeed;			// 移動速度
	vec::Vec3 _targetPosition;	// 目標位置（追跡時の移動先）
	bool _isMoving;				// 移動中かどうか

	// 初期位置に戻る機能
	vec::Vec3 _initialPosition;		// 初期位置
	vec::Vec3 _initialDirection;	// 初期向き
	bool _isReturningToInitialPos;	// 初期位置に戻り中かどうか
	float _returnSpeed;				// 初期位置に戻る速度

	// 検知終了後の待機処理用
	bool _waitingBeforeReturn;     // 帰還前の待機中フラグ
	float _returnWaitTimer;        // 帰還前の待機タイマー
	static constexpr float RETURN_WAIT_TIME = 3.0f; // 待機時間

	//	デバッグ用：YouDiedメッセージ表示関連
	bool _showYouDiedMessage;
	float _youDiedMessageTimer;
	static constexpr float YOU_DIED_DISPLAY_TIME = 2.0f; // 表示時間

	// 敵の向き変更タイマー
	float DirChangeTimer;
	static constexpr float DirChangeInterval = 15.0f; // 向き変更の間隔

	// テレポート関連
	bool _waitingForTeleport;		// テレポート待機中フラグ
	float _teleportTimer;			// テレポートまでの待機時間
	static constexpr float TELEPORT_WAIT_TIME = 3.0f; // テレポートまでの待機時間

	// 音検知による移動関連
	bool _isMovingToSound;			// 音源に向かって移動中かどうか
	vec::Vec3 _soundSourcePosition;	// 検知した音源の位置
	void UpdateMovingToSound();		// 音源に向かって移動する処理

	// 音源到達後の待機処理
	bool _waitingAtSound;			// 音源到達後の待機中フラグ
	float _soundWaitTimer;			// 音源到達後の待機タイマー
	static constexpr float SOUND_WAIT_TIME = 3.0f; // 音源到達後の待機時間

	// 音検知からの経過時間管理
	bool _soundDetectionActive;		// 音検知タイマーが有効かどうか
	float _soundDetectionTimer;		// 音検知からの経過時間
	static constexpr float SOUND_RETURN_TIME = 10.0f; // 音検知から初期位置に戻るまでの時間

	at::spc<EffectBase> _effect;

	bool _isInvincible;	// 無敵状態かどうか
	float _stanTimer; // スタン時間
	int _attachStage; // ダメージ後のアニメーション名
	static constexpr float STAN_DURATION = 60.0f; // スタン時間

	// アニメーション名
	std::string _attachAnimDamage;// ダメージアニメーション名
	std::string _attachAnimStan;  // スタンアニメーション名
	std::string _attachAnimGetUp; // 起き上がりアニメーション名
};