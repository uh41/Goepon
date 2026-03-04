/*********************************************************************/
// * \file   enemybase.cpp
// * \brief  エネミーベースクラス（共通処理集中）
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "enemybase.h"
#include "enemysensor.h"
#include "applicationglobal.h"
#include "enemysoundmanager.h"

// 初期化
bool EnemyBase::Initialize()
{
	base::Initialize();

	_handle = -1;
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

	// センサー関連の初期化
	_detectedPlayer = false;					// プレイヤー検出フラグの初期化
	_playerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	// プレイヤー位置の初期化
	_rotationSpeed = 0.5f;						// 回転速度（調整可能）

	// 移動関連の初期化
	_moveSpeed = 2.0f;								// 移動速度（調整可能）
	_targetPosition = vec3::VGet(0.0f, 0.0f, 0.0f);	// 目標位置の初期化
	_isMoving = false;								// 移動中フラグの初期化

	// 初期位置に戻る機能の初期化
	_isReturningToInitialPos = false;	// 初期位置に戻り中フラグの初期化
	_returnSpeed = 1.5f;				// 初期位置に戻る速度（追跡より少し遅め）

	// テレポート関連の初期化
	_waitingForTeleport = false;
	_teleportTimer = 0.0f;

	_waitingBeforeReturn = false;
	_returnWaitTimer = 0.0f;

	// 音検知による移動の初期化
	_isMovingToSound = false;
	_soundSourcePosition = vec3::VGet(0.0f, 0.0f, 0.0f);

	// 音源到達後の待機処理の初期化
	_waitingAtSound = false;
	_soundWaitTimer = 0.0f;

	// 音検知からの経過時間管理の初期化
	_soundDetectionActive = false;
	_soundDetectionTimer = 0.0f;

	// 攻撃無力化フラグの初期化
	_isInvincible = false;
	_stanTimer = 0.0f;
	_attachStage = 0;
	_attachAnimDamage = "";
	_attachAnimStan = "";
	_attachAnimGetUp = "";

	_effect = nullptr;

	// 向き変更のシーケンス関連の初期化
	_dirSequence.clear();
	_dirSeqIndex = 0;
	_dirSeqTimer = 0.0f;
	_dirSeqWaitTime = 0.0f;
	_dirSeqActive = false;

	_playSightOffOnReturn = false;

	return true;
}

// 終了
bool EnemyBase::Terminate()
{
	base::Terminate();
	return true;
}

// 初期位置と初期向きをキャプチャする関数
void EnemyBase::CaptureInitialTransform()
{
	_initialPosition = _vPos;
	_initialDirection = _vDir;
}

// テレポート状態のリセット
void EnemyBase::ResetTeleport()
{
	_waitingForTeleport = false;
	_teleportTimer = 0.0f;
}

// プレイヤーが検出された時の処理
void EnemyBase::OnPlayerDetected(const vec::Vec3& playerPos)
{
	// 初期位置に戻り中は検出を無視
	if(_isReturningToInitialPos)
	{
		return;
	}

	// 変更点：遷移検出のため、呼び出し前の状態を保存
	bool wasDetected = _detectedPlayer;

	_detectedPlayer = true;	// プレイヤーを検出したフラグを立てる
	_playerPos = playerPos;	// 検出したプレイヤーの位置を保存

	// プレイヤーを検出したら初期位置に戻るのを中断
	_isReturningToInitialPos = false;

	// テレポート関連をリセット
	ResetTeleport();

	// 音源への移動と待機を中断
	_isMovingToSound = false;
	_waitingAtSound = false;
	_soundWaitTimer = 0.0f;

	// 音検知タイマーをリセット
	_soundDetectionActive = false;
	_soundDetectionTimer = 0.0f;

	// 遷移（未検出 -> 検出）時のみ効果音を鳴らす
	if(!wasDetected)
	{
		if(gGlobal._soundServer)
		{
			auto bushfound = gGlobal._soundServer->Get("35");
			if(bushfound && !bushfound->IsPlay())
			{
				bushfound->Play();
			}
		}
	}
}

// EnemySensorを設定
void EnemyBase::SetEnemySensor(std::shared_ptr<EnemySensor> sensor)
{
	_enemySensor = sensor;
}

// プレイヤーが検出範囲外になった時の処理
void EnemyBase::OnPlayerLost()
{
	// 既に検出していない場合は何もしない
	if(_detectedPlayer)
	{
		_detectedPlayer = false;
		_isMoving = false;

		// 検知終了後、すぐに戻らず待機状態にする
		_waitingBeforeReturn = true;
		_returnWaitTimer = RETURN_WAIT_TIME;

		// センサーの追跡状態をリセット
		if(_enemySensor)
		{
			_enemySensor->ResetDetection();
		}
	}
}

// 初期位置に戻る処理を開始
void EnemyBase::StartReturningToInitialPosition()
{
	// 既に初期位置にいる場合は何もしない
	if(!IsAtInitialPosition())
	{
		_isReturningToInitialPos = true; // 初期位置に戻り始める
		_isMoving = false;				 // 他の移動を停止

		// テレポート関連をリセット
		ResetTeleport();

		// 初期位置に戻り始める際に検出状態をリセット
		_detectedPlayer = false;

		// EnemySensorの検出状態もリセット
		if(_enemySensor)
		{
			_enemySensor->ResetDetection();
		}

		// 音源への移動と待機を中断
		_isMovingToSound = false;
		_waitingAtSound = false;
		_soundWaitTimer = 0.0f;

		// 音検知タイマーをリセット
		_soundDetectionActive = false;
		_soundDetectionTimer = 0.0f;

		_effect->PlayEffect(_vPos); // 戻り始める位置でエフェクトを再生
	}
}

// 床の存在を確認する関数
bool EnemyBase::CheckFloorExistence(const vec::Vec3& position)
{
	// EnemySensorが設定されている場合はそれを使用
	if(_enemySensor)
	{
		return _enemySensor->CheckFloorExistence(position);
	}

	// EnemySensorがない場合は常にtrue（元の動作を維持）
	return true;
}

// プレイヤーの方向に徐々に回転する処理
void EnemyBase::UpdateRotationToPlayer()
{
	vec::Vec3 targetPos;

	if(IsStun())
	{
		// スタン中は向きを変えない
		return;
	}

	// 追跡中か検出中かでターゲット位置を決定
	if(_enemySensor && _enemySensor->IsChasing())
	{
		// 追跡中は最後に確認されたプレイヤーの位置を使用
		targetPos = _enemySensor->GetLastKnownPlayerPosition();
	}
	else if(_detectedPlayer)
	{
		// 通常の検出時は現在のプレイヤー位置を使用
		targetPos = _playerPos;
	}
	else
	{
		// 検出もしていないし追跡もしていない場合は何もしない
		return;
	}

	// プレイヤーへの方向ベクトルを計算
	vec::Vec3 toPlayer = vec3::VSub(targetPos, _vPos);	// ターゲット位置へのベクトル
	toPlayer.y = 0.0f;									// Y成分は無視して水平方向のみ

	// 距離が0に近い場合は何もしない
	if(vec3::VSize(toPlayer) < 0.01f) return;

	// ターゲット方向を正規化
	vec::Vec3 targetDir = vec3::VNorm(toPlayer);

	// 現在の向きとターゲット方向の角度差を計算
	float currentAngle = atan2f(_vDir.x, _vDir.z);			// 現在の向きの角度
	float targetAngle = atan2f(targetDir.x, targetDir.z);	// ターゲット方向の角度

	// 角度差を計算（-π から π の範囲に正規化）
	float angleDiff = targetAngle - currentAngle;				// 角度差
	while(angleDiff > DX_PI_F) angleDiff -= 2.0f * DX_PI_F;	// 正規化
	while(angleDiff < -DX_PI_F) angleDiff += 2.0f * DX_PI_F;	// 正規化

	// 回転速度を制限
	if(abs(angleDiff) > _rotationSpeed)
	{
		angleDiff = (angleDiff > 0) ? _rotationSpeed : -_rotationSpeed;
	}

	// 新しい角度を計算
	float newAngle = currentAngle + angleDiff;
	_vDir.x = sin(newAngle);
	_vDir.z = cos(newAngle);
}

// プレイヤーの方向を向く処理（即座に向く）
void EnemyBase::LookAtPlayer()
{
	if(!_detectedPlayer) return;

	if(IsStun())
	{
		// スタン中は向きを変えない
		return;
	}

	// プレイヤーへの方向ベクトルを計算
	vec::Vec3 toPlayer = vec3::VSub(_playerPos, _vPos);

	// Y成分は無視して水平方向のみ
	toPlayer.y = 0.0f;

	// 距離が0でないことを確認
	if(vec3::VSize(toPlayer) > 0.01f)
	{
		// 正規化して向きを設定
		_vDir = vec3::VNorm(toPlayer);
	}
}

// 初期位置にいるかどうかをチェック
bool EnemyBase::IsAtInitialPosition() const
{
	// 現在位置と初期位置の距離を計算
	float distance = vec3::VSize(vec3::VSub(_vPos, _initialPosition));
	return distance < 30.0f; // 30.0f以内なら初期位置とみなす
}

void EnemyBase::StartDamage()
{
	// 既に無敵状態なら何もしない
	if(_isInvincible) { return; }

	_isInvincible = true;
	_attachStage = 1;// ダメージステージに移行
	_stanTimer = 0.0f;
	_isReturningToInitialPos = false;
	_waitingForTeleport = false;
	_teleportTimer = 0.0f;

	// ダメージ開始時の処理
	OnDamageStart();

	if(_enemySensor)
	{
		_enemySensor->SetSensorEnabled(false); // センサーを無効化
	}

	_isMoving = false;			// 移動を停止
	_isMovingToSound = false;	// 音源への移動も停止
	_waitingAtSound = false;	// 音源到達後の待機も停止
	_detectedPlayer = false;	// プレイヤー検出もリセット

	// ダメージアニメーションの再生
	if(!_attachAnimDamage.empty())
	{
		_animId = PlayAnimation(_attachAnimDamage, false);
		_fPlayTime = 0.0f;
	}
}

// ダメージアニメーションの更新
void EnemyBase::UpdateDamageAnimation()
{
	if(!_isInvincible) { return; }

	if(_attachStage == 1)
	{
		if(_animId == -1 || !AnimationManager::GetInstance()->IsPlaying(_animId))
		{
			_attachStage = 2;
			_stanTimer = STAN_DURATION;

			// プレイヤーの攻撃SE
			auto sound = gGlobal._soundServer->Get("30");
			if(sound && !sound->IsPlay())
			{
				sound->Play();
			}

			// 転ばされたときのボイス
			auto damageSound = gGlobal._soundServer->Get("32");
			if(damageSound && !damageSound->IsPlay())
			{
				damageSound->Play();
			}

			EnemySoundManager::GetInstance()->EmitSound(_vPos, 5, 300.0f, 50.0f); // ダメージ音を発生させる

			if(!_attachAnimStan.empty())
			{
				_animId = PlayAnimation(_attachAnimStan, true);
				_fPlayTime = 0.0f;
			}
		}
	}
	else if(_attachStage == 2)
	{
		_stanTimer -= 1.0f / 60.0f; // 60FPSとして計算

		if(_stanTimer <= 0.0f)
		{
			_attachStage = 3;
			if(_animId != -1)
			{
				AnimationManager::GetInstance()->Stop(_animId);	
				_animId = -1;
			}

			if(!_attachAnimGetUp.empty())
			{
				_animId = PlayAnimation(_attachAnimGetUp, false);
				_fPlayTime = 0.0f;
				// 起き上がりのボイス
				auto getUpSound = gGlobal._soundServer->Get("34");
				if(getUpSound && !getUpSound->IsPlay())
				{
					getUpSound->Play();
				}
			}
		}
	}
	else if(_attachStage == 3)
	{
		if(_animId == -1 || !AnimationManager::GetInstance()->IsPlaying(_animId))
		{
			_attachStage = 0;
			_isInvincible = false;

			if(_enemySensor)
			{
				_enemySensor->SetSensorEnabled(true);
				_enemySensor->ResetDetection();
			}

			// ダメージ終了時の処理
			OnDamageEnd();
		}
	}
}

// 初期位置に戻る更新処理
void EnemyBase::UpdateReturningToInitialPosition()
{
	if(!_isReturningToInitialPos) return;

	// 初期位置に戻り中は常に検出状態をfalseに保つ
	_detectedPlayer = false;

	// テレポート待機中の場合
	if(_waitingForTeleport)
	{
		_teleportTimer -= 1.0f / 60.0f; // 60FPSとして計算

		// タイマーが0以下になったらテレポート実行
		if(_teleportTimer <= 0.0f)
		{
			// 3秒経過したので初期位置にテレポート
			_effect->PlayEffect(_vPos); // テレポート前のエフェクト
			_vPos = _initialPosition;
			if(!IsStun())
			{
				_vDir = _initialDirection;
			}
			_isReturningToInitialPos = false;	// 初期位置に戻り完了
			_waitingForTeleport = false;		// テレポート待機終了
			_teleportTimer = 0.0f;				// テレポート完了後のタイマーリセット
			_effect->PlayEffect(_vPos);			// テレポート後のエフェクト
		}
		return;
	}

	// 初期位置への方向ベクトルを計算
	vec::Vec3 toInitialPos = vec3::VSub(_initialPosition, _vPos);
	toInitialPos.y = 0.0f; // Y軸は無視

	float distance = vec3::VSize(toInitialPos);	// 初期位置までの距離

	// 初期位置に十分近い場合
	if(distance < 30.0f)
	{
		_vPos = _initialPosition;
		_vDir = _initialDirection;
		_isReturningToInitialPos = false;

		_detectedPlayer = false;
		if(_enemySensor)
		{
			_enemySensor->ResetDetection();	// センサーの検出状態もリセット
		}

		if(_playSightOffOnReturn && gGlobal._soundServer)
		{
			auto sightOff = gGlobal._soundServer->Get("33");
			if(sightOff && !sightOff->IsPlay())
			{
				sightOff->Play();	// 初期位置に戻ったときのSEを再生
			}
		}
		_playSightOffOnReturn = false;

		return;
	}

	// 正規化して移動方向を取得
	vec::Vec3 moveDirection = vec3::VNorm(toInitialPos);

	// 移動量を計算
	vec::Vec3 movement = vec3::VScale(moveDirection, _returnSpeed);

	// 新しい位置を計算
	vec::Vec3 newPos = vec3::VAdd(_vPos, movement);

	// 床の存在を確認してから移動
	if(CheckFloorExistence(newPos))
	{
		_vPos = newPos;
	}
	else
	{
		// 床がない場合はテレポート待機開始
		if(!_waitingForTeleport)
		{
			_waitingForTeleport = true;
			_teleportTimer = TELEPORT_WAIT_TIME;
		}
	}

	if(!IsStun())
	{
		// 移動方向に向きを徐々に変更
		float currentAngle = atan2f(_vDir.x, _vDir.z);
		float targetAngle = atan2f(moveDirection.x, moveDirection.z);

		// 角度差を計算
		float angleDiff = targetAngle - currentAngle;
		while(angleDiff > DX_PI_F) angleDiff -= 2.0f * DX_PI_F;
		while(angleDiff < -DX_PI_F) angleDiff += 2.0f * DX_PI_F;

		// 回転速度を制限
		if(abs(angleDiff) > _rotationSpeed)
		{
			angleDiff = (angleDiff > 0) ? _rotationSpeed : -_rotationSpeed;
		}

		// 新しい角度を計算
		float newAngle = currentAngle + angleDiff;
		_vDir.x = sin(newAngle);
		_vDir.z = cos(newAngle);

	}
	// 初期位置に戻り中は検出状態を再度falseに設定（念のため）
	_detectedPlayer = false;
}

// 音源に向かって移動する処理
void EnemyBase::UpdateMovingToSound()
{
	if (!_isMovingToSound) return;

	// 音源までの距離を計算
	vec::Vec3 toSound = vec3::VSub(_soundSourcePosition, _vPos);
	toSound.y = 0.0f; // Y軸は無視

	float distance = vec3::VSize(toSound);

	// 音源に十分近づいたら移動を停止
	const float arrivalDistance = 50.0f;
	if (distance < arrivalDistance)
	{
		_isMovingToSound = false;
		_waitingAtSound = true;
		_soundWaitTimer = SOUND_WAIT_TIME;
		_status = STATUS::WAIT;
		return;
	}

	// 音源の方向を向く
	vec::Vec3 soundDir = vec3::VNorm(toSound);
	float currentAngle = atan2f(_vDir.x, _vDir.z);
	float targetAngle = atan2f(soundDir.x, soundDir.z);

	// 角度差を計算（-π から π の範囲に正規化）
	float angleDiff = targetAngle - currentAngle;
	while (angleDiff > DX_PI_F) angleDiff -= 2.0f * DX_PI_F;
	while (angleDiff < -DX_PI_F) angleDiff += 2.0f * DX_PI_F;

	// 回転速度を制限
	if (abs(angleDiff) > _rotationSpeed)
	{
		angleDiff = (angleDiff > 0) ? _rotationSpeed : -_rotationSpeed;
	}

	if(!IsStun())
	{
		// 新しい角度を計算
		float newAngle = currentAngle + angleDiff;
		_vDir.x = sin(newAngle);
		_vDir.z = cos(newAngle);
	}

	// 音源に向かって移動（壁回避機能を使用）
	// 壁回避を試行する角度テーブル（度数）
	static const float wallAvoidanceAngles[] =
	{
		0.0f,     // 直進
		-30.0f,   // 左30度
		30.0f,    // 右30度
		-60.0f,   // 左60度
		60.0f,    // 右60度
		-90.0f,   // 左90度
		90.0f,    // 右90度
		-120.0f,  // 左120度
		120.0f,   // 右120度
	};

	vec::Vec3 finalMovement;
	bool validMovementFound = false;

	// 各角度で移動可能かチェック
	for (int i = 0; i < sizeof(wallAvoidanceAngles) / sizeof(wallAvoidanceAngles[0]); i++)
	{
		float angleRad = DEG2RAD(wallAvoidanceAngles[i]);

		// 音源への方向を指定角度分回転
		vec::Vec3 testDirection;
		testDirection.x = soundDir.x * cos(angleRad) - soundDir.z * sin(angleRad);
		testDirection.y = 0.0f;
		testDirection.z = soundDir.x * sin(angleRad) + soundDir.z * cos(angleRad);

		// 移動量を計算
		vec::Vec3 testMovement = vec3::VScale(testDirection, _moveSpeed);

		// テスト移動後の位置を計算
		vec::Vec3 testPos = vec3::VAdd(_vPos, testMovement);

		// 床の存在を確認
		if (CheckFloorExistence(testPos))
		{
			finalMovement = testMovement;
			validMovementFound = true;

						// 直進以外の方向で移動する場合、その方向を向く
			if (i > 0)
			{
				if(!IsStun())
				{
					// 移動方向に徐々に向きを変更
					float testAngle = atan2f(testDirection.x, testDirection.z);
					float testAngleDiff = testAngle - currentAngle;
					while(testAngleDiff > DX_PI_F) testAngleDiff -= 2.0f * DX_PI_F;
					while(testAngleDiff < -DX_PI_F) testAngleDiff += 2.0f * DX_PI_F;

					// 回転速度を制限
					float maxRotation = _rotationSpeed * 1.5f;
					if(abs(testAngleDiff) > maxRotation)
					{
						testAngleDiff = (testAngleDiff > 0) ? maxRotation : -maxRotation;
					}

					// 新しい角度を適用
					float adjustedAngle = currentAngle + testAngleDiff;
					_vDir.x = sin(adjustedAngle);
					_vDir.z = cos(adjustedAngle);
				}
			}
			break;
		}
	}

	// 移動可能な方向が見つからない場合はその場で待機
	if (!validMovementFound)
	{
		// すぐに初期位置に戻さず、その場で待機して音検知タイマーに任せる
		_status = STATUS::WAIT;
		return;
	}

	// 実際に移動を実行
	_vPos = vec3::VAdd(_vPos, finalMovement);

	// 実際に移動した方向に即座に向ける（壁回避で曲がった場合の向きずれ防止）
	if (vec3::VSize(finalMovement) > 0.001f)
	{
		vec::Vec3 moveDir = vec3::VNorm(finalMovement);
		moveDir.y = 0.0f;
		if(!IsStun())
		{
			_vDir = moveDir;
		}
	}

	// 移動中はWALKステータスに設定
	_status = STATUS::WALK;
}

// 計算処理
bool EnemyBase::Process()
{
	base::Process();

	// ダメージアニメーションの更新
	if(_isInvincible)
	{
		UpdateDamageAnimation();
		return true;
	}

	// プレイヤーの方向に徐々に回転
	UpdateDirectionSequence();

	if(_detectedPlayer || (_enemySensor && _enemySensor->IsChasing()))
	{
		UpdateRotationToPlayer();
	}

	return true;
}

// 目標位置に向かって移動するメソッド（壁回避機能付き）
// もともと Enemy に入っていた長めの実装を共通化して EnemyBase に移しました。
void EnemyBase::MoveTowardsTarget(const vec::Vec3& target)
{
	// 目標位置への方向ベクトルを計算
	vec::Vec3 toTarget = vec3::VSub(target, _vPos);
	toTarget.y = 0.0f; // Y軸は無視（水平移動のみ）

	// 目標位置までの距離
	float distance = vec3::VSize(toTarget);

	// 十分近い場合は移動しない（停止距離を縮小）
	const float stopDistance = 5.0f;
	if(distance < stopDistance)
	{
		_isMoving = false;
		return;
	}

	_isMoving = true;	// 移動中フラグを立てる

	// 正規化して移動方向を取得
	vec::Vec3 moveDirection = vec3::VNorm(toTarget);

	// 壁回避を試行する角度テーブル（度数）
	static const float wallAvoidanceAngles[] = {
		0.0f,     // 直進
		-45.0f,   // 左45度
		45.0f,    // 右45度
		-90.0f,   // 左90度
		90.0f,    // 右90度
		-135.0f,  // 左135度
		135.0f,   // 右135度
		180.0f    // 後退（最後の手段）
	};

	vec::Vec3 finalMovement;		// 最終的な移動量
	bool validMovementFound = false;// 移動可能な方向が見つかったかどうか

	// 各角度で移動可能かチェック
	for(int i = 0; i < sizeof(wallAvoidanceAngles) / sizeof(wallAvoidanceAngles[0]); i++)
	{
		float angleRad = DEG2RAD(wallAvoidanceAngles[i]);	// 角度をラジアンに変換

		// 現在の移動方向を指定角度分回転
		vec::Vec3 testDirection;
		testDirection.x = moveDirection.x * cos(angleRad) - moveDirection.z * sin(angleRad);
		testDirection.y = 0.0f;
		testDirection.z = moveDirection.x * sin(angleRad) + moveDirection.z * cos(angleRad);

		// 移動量を計算
		vec::Vec3 testMovement = vec3::VScale(testDirection, _moveSpeed);

		// テスト移動後の位置を計算
		vec::Vec3 testPos = vec3::VAdd(_vPos, testMovement);

		// 床の存在を確認
		if(CheckFloorExistence(testPos))
		{
			finalMovement = testMovement;
			validMovementFound = true;

			// 直進以外の方向で移動する場合、その方向を向く
			if(i > 0)
			{
				if(!IsStun())
				{
					// 移動方向に徐々に向きを変更
					float currentAngle = atan2f(_vDir.x, _vDir.z);
					float targetAngle = atan2f(testDirection.x, testDirection.z);

					// 角度差を計算（-π から π の範囲に正規化）
					float angleDiff = targetAngle - currentAngle;
					while(angleDiff > DX_PI_F) angleDiff -= 2.0f * DX_PI_F;
					while(angleDiff < -DX_PI_F) angleDiff += 2.0f * DX_PI_F;

					// 回転速度を制限
					float maxRotation = _rotationSpeed * 2.0f; // 壁回避時は少し速く回転
					if(abs(angleDiff) > maxRotation)
					{
						angleDiff = (angleDiff > 0) ? maxRotation : -maxRotation;
					}

					// 新しい角度を適用
					float newAngle = currentAngle + angleDiff;
					_vDir.x = sin(newAngle);
					_vDir.z = cos(newAngle);
				}
			}
			break;
		}
	}

	// 移動可能な方向が見つからない場合は停止
	if(!validMovementFound)
	{
		_isMoving = false;
		return;
	}

	// 実際に移動を実行
	_vPos = vec3::VAdd(_vPos, finalMovement);

	// 目標位置を更新
	_targetPosition = target;
}

// 追跡処理のメソッド
void EnemyBase::UpdateChasing()
{
	// 追跡中かどうかをチェック
	if(_enemySensor && _enemySensor->IsChasing())
	{
		// 追跡中の場合、最後に確認されたプレイヤーの位置に向かって移動
		vec::Vec3 targetPos = _enemySensor->GetLastKnownPlayerPosition();
		MoveTowardsTarget(targetPos);

		// プレイヤーの方向に徐々に向く
		UpdateRotationToPlayer();

		// 初期位置に戻るのを停止
		_isReturningToInitialPos = false;

		// 音源への移動を中断
		_isMovingToSound = false;

		// 音源への移動と待機を中断
		_isMovingToSound = false;
		_waitingAtSound = false;
		_soundWaitTimer = 0.0f;
	}
	else
	{
		// 追跡していない場合は停止
		_isMoving = false;
	}
}


// 描画処理
bool EnemyBase::Render()
{
	base::Render();

	RenderDamageTime();
	return true;
}

// プレイヤーを追跡中かどうかを返すメソッド
bool EnemyBase::IsPlayerChasing() const
{
	return (_enemySensor && _enemySensor->IsChasing());
}

// ダメージ時間の描画処理
void EnemyBase::RenderDamageTime()
{
	// 無敵＋スタンステージ(2) のときのみ表示
	if(!(_isInvincible && _attachStage == 2)) return;
	
	// フォントサイズを設定（必要に応じて変更可）
	SetFontSize(16);
	
	//// 画面左上に表示（位置は調整可）
	DrawFormatString(10, 40, GetColor(255, 200, 0),
	"STAN残り時間: %.1f", _stanTimer);
}

// 方向IDをベクトルに変換するヘルパー関数
vec::Vec3 EnemyBase::DirIdToVec3(int id)
{
	switch(id)
	{
	case 1: return vec::Vec3(0.0f, 0.0f, -1.0f);   // 上
	case 2: return vec::Vec3(1.0f, 0.0f, 0.0f);    // 右
	case 3: return vec::Vec3(0.0f, 0.0f, 1.0f);    // 下
	case 4: return vec::Vec3(-1.0f, 0.0f, 0.0f);   // 左
	default: return vec::Vec3(0.0f, 0.0f, -1.0f);  // 無効なID
	}

}

// 方向IDのシーケンスを設定するメソッド
void EnemyBase::SetDirSequence(const at::vet<int>& sequence, float waitTime)
{
	// シーケンスが空の場合はシーケンスを無効化
	if(sequence.empty())
	{
		_dirSequence.clear();
		_dirSeqActive = false;
		return;
	}

	_dirSequence = sequence;	// シーケンスを保存
	_dirSeqIndex = 0;			// 待機時間を設定（0以下なら待機なし）

	// 待機時間を設定
	if(waitTime > 0.0f)
	{
		_dirSeqWaitTime = waitTime;
	}
	else
	{
		_dirSeqWaitTime = 0.0f;
	}

	_dirSeqTimer = _dirSeqWaitTime;
	_dirSeqActive = true;

	int id = _dirSequence[_dirSeqIndex];	// 最初の方向IDをベクトルに変換して設定
	_vDir = DirIdToVec3(id);				// 最初の方向を設定
}

// JSONから方向IDのシーケンスを設定するメソッド
void EnemyBase::SetDirSequenceFromJson(const nlohmann::json& j)
{
	if(!j.is_object()) { return; }

	at::vet<int> seq;
	if(j.contains("direction"))
	{
		auto& node = j.at("direction");

		// 配列であれば順番にIDを読み取る
		if(node.is_array())
		{
			for(auto&& it : node)
			{
				// 整数であればシーケンスに追加
				if(it.is_number_integer())
				{
					seq.push_back(it.get<int>());
				}
			}
		}
		else if(node.is_string())
		{
			std::string s = node.get<std::string>();
			size_t pos = 0;
			while(true)
			{
				size_t comma = s.find(',', pos);
				std::string token;
				// カンマが見つからなければ残り全てを、見つかればカンマまでを取り出す
				if(comma == std::string::npos)
				{
					token = s.substr(pos);
				}
				else
				{
					token = s.substr(pos, comma - pos);
				}
				// 空でなければシーケンスに追加
				if(!token.empty())
				{
					seq.push_back(std::stoi(token));// 文字列を整数に変換してシーケンスに追加
				}
				if(comma == std::string::npos) { break; }
				pos = comma + 1;
			}
		}

		if(j.contains("waittime"))
		{
			auto& w = j.at("waittime");
			if(w.is_number())
			{
				_dirSeqWaitTime = w.get<float>();
			}
			else if(w.is_string())
			{
				// 文字列からの変換で例外が発生する可能性があるため安全に扱う
				std::string s = w.get<std::string>();
				try
				{
					// std::stof は不正な文字列で std::invalid_argument を投げる
					// 例外が発生した場合は既存の _dirSeqWaitTime を維持する
					_dirSeqWaitTime = std::stof(s);
				}
				catch(...)
				{
					//何もしない
				}
			}
		}
	}

	if(!seq.empty())
	{
		SetDirSequence(seq, _dirSeqWaitTime);
	}
}

// 方向IDのシーケンスを更新するメソッド
void EnemyBase::UpdateDirectionSequence()
{
	if(!_dirSeqActive || _dirSequence.empty()) {
		return;
	}

	// スタン中は向きを変えない
	if(IsStun()) {
		return;
	}

	// シーケンスはプレイヤー検知中や追跡中には進めない
	if(_detectedPlayer || (_enemySensor && _enemySensor->IsChasing()))
	{
		return;
	}

	// 音源への移動中や待機中は進めない
	if(_isMovingToSound || _waitingAtSound || _isReturningToInitialPos)
	{
		return;
	}

	float deltaTime = 1.0f / 60.0f; // 60FPSとして計算
	_dirSeqTimer -= deltaTime;

	if(_dirSeqTimer <= 0.0f)
	{
		_dirSeqIndex = (_dirSeqIndex + 1) % _dirSequence.size(); // シーケンスをループ
		int id = _dirSequence[_dirSeqIndex];
		vec::Vec3 newDir = DirIdToVec3(id);

		// 向きを正規化して設定
		if(vec3::VSize(newDir) > 0.01f)
		{
			_vDir = vec3::VNorm(newDir);
		}

		_dirSeqTimer = _dirSeqWaitTime; // タイマーをリセット
	}
}

// 音源の位置と音レベルに応じて処理を開始するメソッド
void EnemyBase::StartMoveToSoundFromManager(const vec::Vec3& soundPos, int soundLevel)
{
	// プレイヤー追跡中は音検知を無視
	if (_detectedPlayer || (_enemySensor && _enemySensor->IsChasing()))
	{
		return;
	}

	// 音レベルごとに異なる処理を実行
	switch (soundLevel)
	{
	case 1:
		// レベル1: 小さな音（例：足音）- 向きだけ変える
		if (!_detectedPlayer && !_isReturningToInitialPos && !_isMovingToSound)
		{
			vec::Vec3 toSound = vec3::VSub(soundPos, _vPos);
			toSound.y = 0.0f;
			if (vec3::VSize(toSound) > 0.01f)
			{
				_vDir = vec3::VNorm(toSound);
			}
		}
		break;

	case 2:
		// レベル2: 中程度の音 - 短時間待機して様子見
		if (!_detectedPlayer && !_isReturningToInitialPos)
		{
			_waitingAtSound = true;
			_soundWaitTimer = 1.0f; // 1秒だけ待機
			_status = STATUS::WAIT;
		}
		break;

	case 3:
		// レベル3: やや大きな音 - 音源の方向を向いて待機
		if (!_detectedPlayer && !_isReturningToInitialPos)
		{
			vec::Vec3 toSound = vec3::VSub(soundPos, _vPos);
			toSound.y = 0.0f;
			if (vec3::VSize(toSound) > 0.01f)
			{
				_vDir = vec3::VNorm(toSound);
			}
			_waitingAtSound = true;
			_soundWaitTimer = 2.0f; // 2秒待機
			_status = STATUS::WAIT;
		}
		break;

	case 4:
		// レベル4: 大きな音 - 音源に近づく（距離制限あり）
		if (!_detectedPlayer)
		{
			float distance = vec3::VSize(vec3::VSub(soundPos, _vPos));
			if (distance < 500.0f) // 500.0f以内なら反応
			{
				_detectedPlayer = false;
				_isReturningToInitialPos = false;

				if (_enemySensor)
				{
					_enemySensor->ResetDetection();
				}

				ResetTeleport();

				_isMovingToSound = true;
				_soundSourcePosition = soundPos;

				_soundDetectionActive = true;
				_soundDetectionTimer = 0.0f;

				_waitingAtSound = false;
				_soundWaitTimer = 0.0f;
			}
		}
		break;

	case 5:
		// レベル5: 非常に大きな音 - 優先的に音源に向かう（既存の動作）
		_detectedPlayer = false;
		_isReturningToInitialPos = false;

		if (_enemySensor)
		{
			_enemySensor->ResetDetection();
		}

		ResetTeleport();

		_isMovingToSound = true;
		_soundSourcePosition = soundPos;

		_soundDetectionActive = true;
		_soundDetectionTimer = 0.0f;

		_waitingAtSound = false;
		_soundWaitTimer = 0.0f;
		break;

	default:
		// 未定義の音レベルは無視
		break;
	}
	//現在はレベル５のみ機能している
}
