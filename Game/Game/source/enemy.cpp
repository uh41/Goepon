/*********************************************************************/
// * \file   enemy.cpp
// * \brief  エネミークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "enemy.h"
#include "enemysensor.h"

// 初期化
bool Enemy::Initialize()
{
	base::Initialize();

	_iHandle = MV1LoadModel("res/PoorEnemyMelee/PoorEnemy.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	_vPos = VGet(100.0f, 0.0f, 0.0f);
	_vDir = VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 40.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 10.0f;

	_fHp = 30.0f;

	// センサー関連の初期化
	_detectedPlayer = false;
	_playerPos = VGet(0.0f, 0.0f, 0.0f);
	_rotationSpeed = 0.5f; // 回転速度（調整可能）

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
void Enemy::OnPlayerDetected(const VECTOR& playerPos)
{
	_detectedPlayer = true;
	_playerPos = playerPos;
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
	VECTOR toPlayer = VSub(_playerPos, _vPos);
	// Y成分は無視して水平方向のみ
	toPlayer.y = 0.0f;

	// 距離が0でないことを確認
	if (VSize(toPlayer) > 0.01f)
	{
		// 正規化して向きを設定
		_vDir = VNorm(toPlayer);
	}
}

// プレイヤーの方向に徐々に回転する処理
void Enemy::UpdateRotationToPlayer()
{
	if (!_detectedPlayer) return;

	// プレイヤーへの方向ベクトルを計算
	VECTOR toPlayer = VSub(_playerPos, _vPos);
	toPlayer.y = 0.0f;

	if (VSize(toPlayer) < 0.01f) return;

	// ターゲット方向を正規化
	VECTOR targetDir = VNorm(toPlayer);

	// 現在の向きとターゲット方向の角度差を計算
	float currentAngle = atan2f(_vDir.x, _vDir.z);
	float targetAngle = atan2f(targetDir.x, targetDir.z);

	// 角度差を計算（-π から π の範囲に正規化）
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

	if (_status == STATUS::NONE)
	{
		_status = STATUS::WAIT;
	}

	// EnemySensorがあれば、そのセンサーの位置と向きを自分の位置に同期
	if (_enemySensor)
	{
		_enemySensor->SetPos(_vPos);
		_enemySensor->SetDir(_vDir);
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
			MV1DetachAnim(_iHandle, _iAttachIndex);
			_iAttachIndex = -1;
		}
		// ステータスに応じたアニメーションをアタッチする
		switch (_status)
		{
		case STATUS::WAIT:
		{
			int animIndex = MV1GetAnimIndex(_iHandle, "idle");
			if (animIndex != -1)
			{
				_iAttachIndex = MV1AttachAnim(_iHandle, animIndex, -1, FALSE);
				if (_iAttachIndex != -1)
				{
					_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, _iAttachIndex);
					_fPlayTime = (float)(rand() % 30); // 少しずらす
				}
			}
			break;
		}
		}
		// アタッチしたアニメーションの総再生時間を取得する
		if (_iAttachIndex != -1)
		{
			_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, _iAttachIndex);
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

// 描画処理
bool Enemy::Render()
{
	base::Render();
	// 再生時間をセット
	MV1SetAttachAnimTime(_iHandle, _iAttachIndex, _fPlayTime);

	// 位置
	MV1SetPosition(_iHandle, _vPos);
	// 向きからY軸回転を算出
	VECTOR vrot = { 0,0,0, };
	vrot.y = atan2f(-_vDir.x, -_vDir.z);
	MV1SetRotationXYZ(_iHandle, vrot);

	// 描画
	MV1DrawModel(_iHandle);

	return true;
}