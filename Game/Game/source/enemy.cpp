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

	_handle = MV1LoadModel("res/PoorEnemyMelee/bushi_0114taiki.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	//_vPos = vec3::VGet(100.0f, 0.0f, 0.0f);
	//_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
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

	// テレポート関連の初期化
	_waitingForTeleport = false;
	_teleportTimer = 0.0f;

	// YouDiedメッセージ関連の初期化
	_showYouDiedMessage = false;
	_youDiedMessageTimer = 0.0f;

	// 敵の向き変更タイマーの初期化
	DirChangeTimer = DirChangeInterval;

	return true;
}

void Enemy::CaptureInitialTransform()
{
	_initialPosition = _vPos;
	_initialDirection = _vDir;
}

// 終了
bool Enemy::Terminate()
{
	base::Terminate();
	return true;
}

// テレポート状態のリセット
void Enemy::ResetTeleport()
{
	_waitingForTeleport = false;
	_teleportTimer = 0.0f;
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

	// テレポート関連をリセット
	ResetTeleport();
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

// 床の存在を確認する関数
bool Enemy::CheckFloorExistence(const vec::Vec3& position)
{
	// EnemySensorが設定されている場合はそれを使用
	if (_enemySensor)
	{
		return _enemySensor->CheckFloorExistence(position);
	}

	// EnemySensorがない場合は常にtrue（元の動作を維持）
	return true;
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
		// 検出もしていないし追跡もしていない場合は何もしない
		return; 
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

		// テレポート関連をリセット
		ResetTeleport();

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

	// 初期位置に戻り中は常に検出状態をfalseに保つ
	_detectedPlayer = false;

	// テレポート待機中の場合
	if (_waitingForTeleport)
	{
		_teleportTimer -= 1.0f / 60.0f; // 60FPSとして計算

		// タイマーが0以下になったらテレポート実行
		if (_teleportTimer <= 0.0f)
		{
			// 3秒経過したので初期位置にテレポート
			_vPos = _initialPosition;
			_vDir = _initialDirection;
			_isReturningToInitialPos = false;
			_waitingForTeleport = false;
			_teleportTimer = 0.0f;
		}
		return;
	}

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

		// ここで検出状態をリセット（初期位置に完全に到達してから）
		_detectedPlayer = false;
		if (_enemySensor)
		{
			_enemySensor->ResetDetection();
		}

		return;
	}

	// 正規化して移動方向を取得
	vec::Vec3 moveDirection = vec3::VNorm(toInitialPos);

	// 移動量を計算
	vec::Vec3 movement = vec3::VScale(moveDirection, _returnSpeed);

	// 新しい位置を計算
	vec::Vec3 newPos = vec3::VAdd(_vPos, movement);

	// 床の存在を確認してから移動
	if (CheckFloorExistence(newPos))
	{
		_vPos = newPos;
	}
	else
	{
		// 床がない場合はテレポート待機開始
		if (!_waitingForTeleport)
		{
			_waitingForTeleport = true;
			_teleportTimer = TELEPORT_WAIT_TIME;
		}
	}

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

	// 初期位置に戻り中は検出状態を再度falseに設定（念のため）
	_detectedPlayer = false;
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

			// テレポート関連をリセット
			//ResetTeleport();
		}
		else if (_isReturningToInitialPos)
		{
			// 初期位置に戻り中
			_status = _waitingForTeleport ? STATUS::WAIT : STATUS::WALK;
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
			MV1DetachAnim(_handle, static_cast<int>(_iAttachIndex));
			_iAttachIndex = -1;
		}
		// ステータスに応じたアニメーションをアタッチする
		switch (_status)
		{
		case STATUS::WAIT:
		{
			int animIndex = MV1GetAnimIndex(_handle, "taiki");
			if (animIndex != -1)
			{
				_iAttachIndex = static_cast<float>(MV1AttachAnim(_handle, animIndex, -1, FALSE));
				if (_iAttachIndex != -1)
				{
					_fTotalTime = MV1GetAttachAnimTotalTime(_handle, static_cast<int>(_iAttachIndex));
					_fPlayTime = (float)(rand() % 30); // 少しずらす
				}
			}
			break;
		}
		case STATUS::WALK:
		{
			int animIndex = MV1GetAnimIndex(_handle, "walk");
			if (animIndex != -1)
			{
				_iAttachIndex = static_cast<float>(MV1AttachAnim(_handle, animIndex, -1, FALSE));
				if (_iAttachIndex != -1)
				{
					_fTotalTime = MV1GetAttachAnimTotalTime(_handle, static_cast<int>(_iAttachIndex));
					_fPlayTime = (float)(rand() % 30); // 少しずらす
				}
			}
			break;
		}
		}
		// アタッチしたアニメーションの総再生時間を取得する
		if (_iAttachIndex != -1)
		{
			_fTotalTime = MV1GetAttachAnimTotalTime(_handle, static_cast<int>(_iAttachIndex));
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

	// YouDiedメッセージのタイマー更新
	if (_showYouDiedMessage)
	{
		_youDiedMessageTimer -= 1.0f / 60.0f; 

		// タイマーが0以下になったらメッセージ非表示
		if (_youDiedMessageTimer <= 0.0f)
		{
			_showYouDiedMessage = false;
		}
	}

	// 定期的に方向を90度変える処理
	DirChangeTimer -= 1.0f / 60.0f;

	// タイマーが0以下になったら方向を変える
	if (DirChangeTimer <= 0.0f)
	{
		// タイマーをリセット（15秒ごとに変更）
		DirChangeTimer = 15.0f;	

		// プレイヤーを検出していない、かつ追跡中でもない、かつ初期位置に戻り中でもない場合のみ回転
		if (!_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()) && !_isReturningToInitialPos)
		{
			// 回転先の方向を計算
			float currentAngle = atan2f(_vDir.x, _vDir.z);
			float newAngle = currentAngle + DX_PI_F / 2.0f;

			// 新しい向きベクトルを計算
			vec::Vec3 newDir;
			newDir.x = sin(newAngle);
			newDir.y = 0.0f;
			newDir.z = cos(newAngle);

			// 新しい方向に少し移動した位置で床の存在を確認
			vec::Vec3 testPos = vec3::VAdd(_vPos, vec3::VScale(newDir, _moveSpeed * 5.0f));

			// 床がある場合のみ回転を実行
			if (CheckFloorExistence(testPos))
			{
				_vDir = newDir;
			}
		}
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

// 目標位置に向かって移動するメソッド（壁回避機能付き）
void Enemy::MoveTowardsTarget(const vec::Vec3& target)
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
	for (int i = 0; i < sizeof(wallAvoidanceAngles) / sizeof(wallAvoidanceAngles[0]); i++)
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
		if (CheckFloorExistence(testPos))
		{
			finalMovement = testMovement;
			validMovementFound = true;

			// 直進以外の方向で移動する場合、その方向を向く
			if (i > 0)
			{
				// 移動方向に徐々に向きを変更
				float currentAngle = atan2f(_vDir.x, _vDir.z);
				float targetAngle = atan2f(testDirection.x, testDirection.z);

				// 角度差を計算（-π から π の範囲に正規化）
				float angleDiff = targetAngle - currentAngle;
				while (angleDiff > DX_PI_F) angleDiff -= 2.0f * DX_PI_F;
				while (angleDiff < -DX_PI_F) angleDiff += 2.0f * DX_PI_F;

				// 回転速度を制限
				float maxRotation = _rotationSpeed * 2.0f; // 壁回避時は少し速く回転
				if (abs(angleDiff) > maxRotation)
				{
					angleDiff = (angleDiff > 0) ? maxRotation : -maxRotation;
				}

				// 新しい角度を適用
				float newAngle = currentAngle + angleDiff;
				_vDir.x = sin(newAngle);
				_vDir.z = cos(newAngle);
			}
			break;
		}
	}

	// 移動可能な方向が見つからない場合は停止
	if (!validMovementFound)
	{
		_isMoving = false;
		return;
	}

	// 実際に移動を実行
	_vPos = vec3::VAdd(_vPos, finalMovement);

	// 目標位置を更新
	_targetPosition = target;
}

// 描画処理
bool Enemy::Render()
{
	base::Render();
	// 再生時間をセット
	MV1SetAttachAnimTime(_handle, static_cast<int>(_iAttachIndex), _fPlayTime);
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// ���f�����W���łǂ��������Ă��邩�Ŏ����ς��(�����-z������Ă���ꍇ)

	MATRIX mRotY = MGetRotY(vorty);

	// 位置
	MV1SetPosition(_handle, DxlibConverter::VecToDxLib(_vPos));
	// 向きからY軸回転を算出
	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); // -90�x�i�K�v�ɉ����ĕ����𔽓]�j

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));

	MATRIX mScale = MGetScale(VGet(1.7f, 1.7f, 1.7f));

	MATRIX m = MGetIdent();

	//m = MMult(m, mRotZ);
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);

	// 描画
	MV1DrawModel(_handle);

	return true;
}

// YouDiedメッセージを表示開始
void Enemy::TriggerYouDiedMessage()
{
	_showYouDiedMessage = true;
	_youDiedMessageTimer = YOU_DIED_DISPLAY_TIME;
}

// YouDiedメッセージの描画処理
void Enemy::RenderYouDiedMessage()
{
	if (!_showYouDiedMessage) return;

	// 画面サイズを取得
	int screenWidth = ApplicationMain::GetInstance()->DispSizeW();
	int screenHeight = ApplicationMain::GetInstance()->DispSizeH();

	// フォントサイズを大きく設定
	SetFontSize(72);

	// 表示テキスト
	const char* youDiedText = "YOU DIED";
	int textWidth = GetDrawStringWidth(youDiedText, static_cast<int>(strlen(youDiedText)));

	// 画面中央に配置
	int x = (screenWidth - textWidth) / 2;
	int y = (screenHeight - 72) / 2; // フォントサイズ分考慮

	// 半透明の黒背景を描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(0, 0, screenWidth, screenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 文字に影をつけて見やすくする
	for (int dx = -2; dx <= 2; dx++)
	{
		for (int dy = -2; dy <= 2; dy++)
		{
			if (dx != 0 || dy != 0)
			{
				DrawString(x + dx, y + dy, youDiedText, GetColor(0, 0, 0));
			}
		}
	}

	// メインの文字（赤色）
	DrawString(x, y, youDiedText, GetColor(255, 0, 0));

	// フォントサイズを元に戻す
	SetFontSize(16);

	// 残り時間表示（デバッグ用、必要に応じてコメントアウト可能）
	DrawFormatString(10, 10, GetColor(255, 255, 0),
		"YOU DIED残り時間: %.1f", _youDiedMessageTimer);
}