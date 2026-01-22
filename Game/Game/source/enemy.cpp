/*********************************************************************/
// * \file   enemy.cpp
// * \brief  エネミークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#include "enemy.h"
#include "enemysensor.h"

// 初期化
bool Enemy::Initialize()
{
	base::Initialize();

	_iHandle = MV1LoadModel("res/PoorEnemyMelee/bushi_0114taiki.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	_vPos = vec3::VGet(100.0f, 0.0f, 0.0f);
	_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 40.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 10.0f;

	_fHp = 30.0f;

	// 初期位置と向きを保存
	_initialPosition = _vPos;
	_initialDirection = _vDir;

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

	return true;
}

// 終了
bool Enemy::Terminate()
{
	base::Terminate();
	return true;
}

// EnemySensorを設定
void Enemy::SetEnemySensor(std::shared_ptr<EnemySensor> sensor)
{
	_enemySensor = sensor;
}

// プレイヤーが検出された時の処理
void Enemy::OnPlayerDetected(const vec::Vec3& playerPos)
{
	// 初期位置に戻り中は検出を無視
	if (_isReturningToInitialPos)
	{
		return;
	}

	_detectedPlayer = true;	// プレイヤーを検出したフラグを立てる
	_playerPos = playerPos;	// 検出したプレイヤーの位置を保存

	// プレイヤーを検出したら初期位置に戻るのを中断
	_isReturningToInitialPos = false;
}

// プレイヤーが検出範囲外になった時の処理
void Enemy::OnPlayerLost()
{
	_detectedPlayer = false;
}

// プレイヤーの方向を向く処理（即座に向く）
void Enemy::LookAtPlayer()
{
	if (!_detectedPlayer) return;

	// プレイヤーへの方向ベクトルを計算
	vec::Vec3 toPlayer = vec3::VSub(_playerPos, _vPos);

	// Y成分は無視して水平方向のみ
	toPlayer.y = 0.0f;

	// 距離が0でないことを確認
	if (vec3::VSize(toPlayer) > 0.01f)
	{
		// 正規化して向きを設定
		_vDir = vec3::VNorm(toPlayer);
	}
}

// プレイヤーの方向に徐々に回転する処理
void Enemy::UpdateRotationToPlayer()
{
	vec::Vec3 targetPos;

	// 追跡中か検出中かでターゲット位置を決定
	if (_enemySensor && _enemySensor->IsChasing())
	{
		// 追跡中は最後に確認されたプレイヤーの位置を使用
		targetPos = _enemySensor->GetLastKnownPlayerPosition();
	}
	else if (_detectedPlayer)
	{
		// 通常の検出時は現在のプレイヤー位置を使用
		targetPos = _playerPos;
	}
	else
	{
		return; // 検出もしていないし追跡もしていない場合は何もしない
	}

	// プレイヤーへの方向ベクトルを計算
	vec::Vec3 toPlayer = vec3::VSub(targetPos, _vPos);	// ターゲット位置へのベクトル
	toPlayer.y = 0.0f;									// Y成分は無視して水平方向のみ

	// 距離が0に近い場合は何もしない
	if (vec3::VSize(toPlayer) < 0.01f) return;

	// ターゲット方向を正規化
	vec::Vec3 targetDir = vec3::VNorm(toPlayer);

	// 現在の向きとターゲット方向の角度差を計算
	float currentAngle = atan2f(_vDir.x, _vDir.z);			// 現在の向きの角度
	float targetAngle = atan2f(targetDir.x, targetDir.z);	// ターゲット方向の角度

	// 角度差を計算（-π から π の範囲に正規化）
	float angleDiff = targetAngle - currentAngle;				// 角度差
	while (angleDiff > DX_PI_F) angleDiff -= 2.0f * DX_PI_F;	// 正規化
	while (angleDiff < -DX_PI_F) angleDiff += 2.0f * DX_PI_F;	// 正規化

	// 回転速度を制限
	if (abs(angleDiff) > _rotationSpeed)
	{
		angleDiff = (angleDiff > 0) ? _rotationSpeed : -_rotationSpeed;
	}

	// 新しい角度を計算
	float newAngle = currentAngle + angleDiff;
	_vDir.x = sin(newAngle);
	_vDir.z = cos(newAngle);
}

// 初期位置に戻る処理を開始
void Enemy::StartReturningToInitialPosition()
{
	// 既に初期位置にいる場合は何もしない
	if (!IsAtInitialPosition())
	{
		_isReturningToInitialPos = true; // 初期位置に戻り始める
		_isMoving = false;				 // 他の移動を停止

		// 初期位置に戻り始める際に検出状態をリセット
		_detectedPlayer = false;

		// EnemySensorの検出状態もリセット
		if (_enemySensor)
		{
			_enemySensor->ResetDetection();
		}
	}
}

// 初期位置にいるかどうかをチェック
bool Enemy::IsAtInitialPosition() const
{
	// 現在位置と初期位置の距離を計算
	float distance = vec3::VSize(vec3::VSub(_vPos, _initialPosition)); 
	return distance < 30.0f; // 30.0f以内なら初期位置とみなす
}

// 初期位置に戻る更新処理
void Enemy::UpdateReturningToInitialPosition()
{
	if (!_isReturningToInitialPos) return;

	// 初期位置への方向ベクトルを計算
	vec::Vec3 toInitialPos = vec3::VSub(_initialPosition, _vPos);
	toInitialPos.y = 0.0f; // Y軸は無視

	float distance = vec3::VSize(toInitialPos);	// 初期位置までの距離

	// 初期位置に十分近い場合
	if (distance < 30.0f)
	{
		_vPos = _initialPosition;
		_vDir = _initialDirection;
		_isReturningToInitialPos = false;
		return;
	}

	// 正規化して移動方向を取得
	vec::Vec3 moveDirection = vec3::VNorm(toInitialPos);

	// 移動量を計算
	vec::Vec3 movement = vec3::VScale(moveDirection, _returnSpeed);

	// 新しい位置を計算
	_vPos = vec3::VAdd(_vPos, movement);

	// 移動方向に向きを徐々に変更
	float currentAngle = atan2f(_vDir.x, _vDir.z);
	float targetAngle = atan2f(moveDirection.x, moveDirection.z);

	// 角度差を計算
	float angleDiff = targetAngle - currentAngle;
	while (angleDiff > DX_PI_F) angleDiff -= 2.0f * DX_PI_F;
	while (angleDiff < -DX_PI_F) angleDiff += 2.0f * DX_PI_F;

	// 回転速度を制限
	if (abs(angleDiff) > _rotationSpeed)
	{
		angleDiff = (angleDiff > 0) ? _rotationSpeed : -_rotationSpeed;
	}

	// 新しい角度を計算
	float newAngle = currentAngle + angleDiff;
	_vDir.x = sin(newAngle);
	_vDir.z = cos(newAngle);
}

// 計算処理
bool Enemy::Process()
{
	base::Process();

	CharaBase::STATUS old_status = _status;

	// ステータスがNONEの場合、WAITに設定
	if (_status == STATUS::NONE)
	{
		_status = STATUS::WAIT;
	}

	// EnemySensorがあれば、そのセンサーの位置と向きを自分の位置に同期
	if (_enemySensor)
	{
		_enemySensor->SetPos(_vPos);
		_enemySensor->SetDir(_vDir);

		// 追跡処理
		UpdateChasing();

		// プレイヤーを検出している、または追跡中の場合のみWALKに設定
		if (_detectedPlayer || _enemySensor->IsChasing())
		{
			_status = STATUS::WALK;
			// プレイヤーを検出中は初期位置に戻るのを停止
			_isReturningToInitialPos = false;
		}
		else if (_isReturningToInitialPos)
		{
			// 初期位置に戻り中
			_status = STATUS::WALK;
			UpdateReturningToInitialPosition();
		}
		else
		{
			// 追跡も初期位置への復帰もしていない場合
			_status = STATUS::WAIT;

			// 追跡が終了して初期位置にいない場合、初期位置に戻り始める
			if (!_enemySensor->IsChasing() && !IsAtInitialPosition())
			{
				StartReturningToInitialPosition();
			}
		}
	}

	// プレイヤーを検出している場合、プレイヤーの方向に徐々に向く
	if (_detectedPlayer)
	{
		UpdateRotationToPlayer(); // 徐々に回転
		// または即座に向きたい場合は LookAtPlayer(); を使用
	}

	// ステータスが変わっていないか？
	if (old_status == _status)
	{
		//再生時間を進める
		_fPlayTime += 0.5f;
		// 再生時間をランダムに揺らがせる
		switch (_status)
		{
		case STATUS::WAIT:
		{
			_fPlayTime += (float)(rand() % 10) / 100.0f;// 0.00 ～ 0.09 の揺らぎ。積算するとずれが起きる
			break;
		}
		}
	}
	else
	{
		// アニメーションがアタッチされていたら、デタッチする
		if (_iAttachIndex != -1)
		{
			MV1DetachAnim(_iHandle, static_cast<int>(_iAttachIndex));
			_iAttachIndex = -1;
		}
		// ステータスに応じたアニメーションをアタッチする
		switch (_status)
		{
		case STATUS::WAIT:
		{
			int animIndex = MV1GetAnimIndex(_iHandle, "taiki");
			if (animIndex != -1)
			{
				_iAttachIndex = static_cast<float>(MV1AttachAnim(_iHandle, animIndex, -1, FALSE));
				if (_iAttachIndex != -1)
				{
					_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, static_cast<int>(_iAttachIndex));
					_fPlayTime = (float)(rand() % 30); // 少しずらす
				}
			}
			break;
		}
		case STATUS::WALK:
		{
			int animIndex = MV1GetAnimIndex(_iHandle, "walk");
			if (animIndex != -1)
			{
				_iAttachIndex = static_cast<float>(MV1AttachAnim(_iHandle, animIndex, -1, FALSE));
				if (_iAttachIndex != -1)
				{
					_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, static_cast<int>(_iAttachIndex));
					_fPlayTime = (float)(rand() % 30); // 少しずらす
				}
			}
			break;
		}
		}
		// アタッチしたアニメーションの総再生時間を取得する
		if (_iAttachIndex != -1)
		{
			_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, static_cast<int>(_iAttachIndex));
		}
		// 再生時間を初期化
		_fPlayTime = 0.0f;
		// 再生時間をランダムにずらす
		switch (_status)
		{
		case STATUS::WAIT:
		{
			_fPlayTime += rand() % 30; // 0 ～ 29 の揺らぎ
			break;
		}
		}
	}

	// 再生時間がアニメーションの総再生時間に達したら再生時間を0に戻す
	if (_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	return true;
}

// 追跡処理のメソッド
void Enemy::UpdateChasing()
{
	// 追跡中かどうかをチェック
	if (_enemySensor && _enemySensor->IsChasing())
	{
		// 追跡中の場合、最後に確認されたプレイヤーの位置に向かって移動
		vec::Vec3 targetPos = _enemySensor->GetLastKnownPlayerPosition();
		MoveTowardsTarget(targetPos);

		// プレイヤーの方向に徐々に向く
		UpdateRotationToPlayer();

		// 初期位置に戻るのを停止
		_isReturningToInitialPos = false;
	}
	else
	{
		// 追跡していない場合は停止
		_isMoving = false;
	}
}

// 目標位置に向かって移動するメソッド
void Enemy::MoveTowardsTarget(const vec::Vec3& target)
{
	// 目標位置への方向ベクトルを計算
	vec::Vec3 toTarget = vec3::VSub(target, _vPos);
	toTarget.y = 0.0f; // Y軸は無視（水平移動のみ）

	float distance = vec3::VSize(toTarget);

	// 十分近い場合は移動しない
	if (distance < 50.0f)
	{
		_isMoving = false;
		return;
	}

	_isMoving = true;

	// 正規化して移動方向を取得
	vec::Vec3 moveDirection = vec3::VNorm(toTarget);

	// 移動量を計算
	vec::Vec3 movement = vec3::VScale(moveDirection, _moveSpeed);

	// 新しい位置を計算
	vec::Vec3 newPos = vec3::VAdd(_vPos, movement);
	_vPos = newPos;

	// 目標位置を更新
	_targetPosition = target;
}

// 描画処理
bool Enemy::Render()
{
	base::Render();
	// 再生時間をセット
	MV1SetAttachAnimTime(_iHandle, static_cast<int>(_iAttachIndex), _fPlayTime);

	// 位置
	MV1SetPosition(_iHandle, VectorConverter::VecToDxLib(_vPos));
	// 向きからY軸回転を算出
	VECTOR vrot = { 0,0,0, };
	vrot.y = atan2f(-_vDir.x, -_vDir.z);
	MV1SetRotationXYZ(_iHandle, vrot);

	// 描画
	MV1DrawModel(_iHandle);

	return true;
}