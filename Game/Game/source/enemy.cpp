/*********************************************************************/
// * \file   enemy.cpp
// * \brief  エネミークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "enemy.h"

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

	// 索敵範囲の初期設定（半径400cm、角度60度）
	SetDetectionSector(400.0f, 60.0f);

	return true;
}

// 終了
bool Enemy::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool Enemy::Process()
{
	base::Process();

	CharaBase::STATUS old_status = _status;

	if(_status == STATUS::NONE)
	{
		_status = STATUS::WAIT;
	}

	// ステータスが変わっていないか？
	if(old_status == _status)
	{
		//再生時間を進める
		_fPlayTime += 0.5f;
		// 再生時間をランダムに揺らがせる
		switch(_status)
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
		if(_iAttachIndex != -1)
		{
			MV1DetachAnim(_iHandle, _iAttachIndex);
			_iAttachIndex = -1;
		}
		// ステータスに応じたアニメーションをアタッチする
		switch(_status)
		{
			case STATUS::WAIT:
			{
				int animIndex = MV1GetAnimIndex(_iHandle, "idle");
				if(animIndex != -1)
				{
					_iAttachIndex = MV1AttachAnim(_iHandle, animIndex, -1, FALSE);
					if(_iAttachIndex != -1)
					{
						_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, _iAttachIndex);
						_fPlayTime = (float)(rand() % 30); // 少しずらす
					}
				}
				break;
			}
		}
		// アタッチしたアニメーションの総再生時間を取得する
		if(_iAttachIndex != -1)
		{
			_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, _iAttachIndex);
		}
		// 再生時間を初期化
		_fPlayTime = 0.0f;
		// 再生時間をランダムにずらす
		switch(_status)
		{
			case STATUS::WAIT:
			{
				_fPlayTime += rand() % 30; // 0 ～ 29 の揺らぎ
				break;
			}
		}
	}

	// 再生時間がアニメーションの総再生時間に達したら再生時間を0に戻す
	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	return true;
}

// 索敵範囲の設定
void Enemy::SetDetectionSector(float radius, float angle)
{
	_detectionSector.radius = radius;
	_detectionSector.angle = angle;
	_bHasDetectionSector = true;
}

// プレイヤーが索敵範囲内にいるかチェック
bool Enemy::IsPlayerInDetectionRange(const VECTOR& playerPos) const
{
	if (!_bHasDetectionSector)
	{
		return false;
	}

	// 索敵範囲情報を一時的な変数として作成
	DetectionSector currentSector;
	currentSector.center = _vPos;
	currentSector.forward = _vDir;
	currentSector.radius = _detectionSector.radius;
	currentSector.angle = _detectionSector.angle;

	// 敵からプレイヤーへのベクトル
	VECTOR toPlayer = VSub(playerPos, currentSector.center);

	// 距離チェック
	float distance = VSize(toPlayer);
	if (distance > currentSector.radius)
	{
		return false; // 範囲外
	}

	// 距離が0に近い場合（同じ位置）は範囲内とみなす
	if (distance < 0.001f)
	{
		return true;
	}

	// 角度チェック
	// プレイヤーへのベクトルを正規化
	VECTOR toPlayerNorm = VNorm(toPlayer);

	// 正面方向との内積を計算して角度を求める
	float dot = VDot(currentSector.forward, toPlayerNorm);

	// 内積から角度を計算（ラジアン）
	float angleBetween = acos(fmaxf(-1.0f, fminf(1.0f, dot)));

	// 度に変換
	float angleDegrees = angleBetween * 180.0f / DX_PI_F;

	// 扇形の半角と比較
	float halfAngle = currentSector.angle * 0.5f;

	return angleDegrees <= halfAngle;
}

// デバッグ用：索敵範囲の描画
void Enemy::RenderDetectionSector() const
{
	if (!_bHasDetectionSector)
	{
		return;
	}

	// 索敵範囲の色（より目立つ色に変更）
	unsigned int color = GetColor(255, 0, 0); // 純粋な赤色

	// 扇形を描画するための分割数
	const int segments = 32;
	const float halfAngle = _detectionSector.angle * 0.5f;

	// 現在の敵の位置と向きを使用
	VECTOR center = _vPos;
	VECTOR forward = _vDir;

	// 正面方向からの角度オフセットを計算
	float forwardAngle = atan2f(-forward.x, -forward.z);

	// 扇形の開始角度と終了角度
	float startAngle = forwardAngle - (halfAngle * DX_PI_F / 180.0f);
	float endAngle = forwardAngle + (halfAngle * DX_PI_F / 180.0f);

	// 扇形の輪郭線を描画（太い線にする）
	for (int i = 0; i < segments; i++)
	{
		float angle1 = startAngle + (endAngle - startAngle) * i / (float)segments;
		float angle2 = startAngle + (endAngle - startAngle) * (i + 1) / (float)segments;

		VECTOR pos1 = VAdd(center, VGet(sinf(angle1) * _detectionSector.radius, 0, cosf(angle1) * _detectionSector.radius));
		VECTOR pos2 = VAdd(center, VGet(sinf(angle2) * _detectionSector.radius, 0, cosf(angle2) * _detectionSector.radius));

		// 3D空間での線描画
		DrawLine3D(pos1, pos2, color);

		// 少し上の位置にも線を描画して見えやすくする
		VECTOR pos1_up = VAdd(pos1, VGet(0, 10.0f, 0));
		VECTOR pos2_up = VAdd(pos2, VGet(0, 10.0f, 0));
		DrawLine3D(pos1_up, pos2_up, color);
	}

	// 中心から両端への線を描画
	VECTOR leftEdge = VAdd(center, VGet(sinf(startAngle) * _detectionSector.radius, 0, cosf(startAngle) * _detectionSector.radius));
	VECTOR rightEdge = VAdd(center, VGet(sinf(endAngle) * _detectionSector.radius, 0, cosf(endAngle) * _detectionSector.radius));

	DrawLine3D(center, leftEdge, color);
	DrawLine3D(center, rightEdge, color);

	// 少し上の位置にも線を描画
	VECTOR center_up = VAdd(center, VGet(0, 10.0f, 0));
	VECTOR leftEdge_up = VAdd(leftEdge, VGet(0, 10.0f, 0));
	VECTOR rightEdge_up = VAdd(rightEdge, VGet(0, 10.0f, 0));

	DrawLine3D(center_up, leftEdge_up, color);
	DrawLine3D(center_up, rightEdge_up, color);

	// 中心点を示すマーカーを描画
	VECTOR marker1 = VAdd(center, VGet(-10.0f, 5.0f, 0));
	VECTOR marker2 = VAdd(center, VGet(10.0f, 5.0f, 0));
	VECTOR marker3 = VAdd(center, VGet(0, 5.0f, -10.0f));
	VECTOR marker4 = VAdd(center, VGet(0, 5.0f, 10.0f));

	DrawLine3D(marker1, marker2, color);
	DrawLine3D(marker3, marker4, color);
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

	// 索敵範囲の描画（デバッグ用）
	RenderDetectionSector();
	return true;
}