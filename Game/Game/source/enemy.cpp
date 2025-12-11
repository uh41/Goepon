#include "enemy.h"

// 初期化
bool Enemy::Initialize()
{
	base::Initialize();

	_handle = MV1LoadModel("res/PoorEnemyMelee/PoorEnemy.mv1");
	_attach_index = -1;
	// ステータスを「無し」に設定
	_status = STATUS::WAIT;
	// 再生時間の初期化
	_total_time = 0.0f;
	_play_time = 0.0f;
	// 位置、向きの初期化
	_pos = VGet(100.0f, 0.0f, 0.0f);
	_dir = VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_col_sub_y = 40.0f;
	// コリジョン半径の設定
	_collision_r = 30.0f;
	_collision_weight = 10.0f;

	_hp = 30.0f;

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

	// ステータスが変わっていないか？
	if(old_status == _status)
	{
		//再生時間を進める
		_play_time += 0.5f;
		// 再生時間をランダムに揺らがせる
		switch(_status)
		{
			case STATUS::WAIT:
			{
				_play_time += (float)(rand() % 10) / 100.0f;// 0.00 ～ 0.09 の揺らぎ。積算するとずれが起きる
				break;
			}
		}
	}
	else
	{
		// アニメーションがアタッチされていたら、デタッチする
		if(_attach_index != -1)
		{
			MV1DetachAnim(_handle, _attach_index);
			_attach_index = -1;
		}
		// ステータスに応じたアニメーションをアタッチする
		switch(_status)
		{
			case STATUS::WAIT:
			{
				int animIndex = MV1GetAnimIndex(_handle, "idle");
				if(animIndex != -1)
				{
					_attach_index = MV1AttachAnim(_handle, animIndex, -1, FALSE);
					if(_attach_index != -1)
					{
						_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);
						_play_time = (float)(rand() % 30); // 少しずらす
					}
				}
				break;
			}
		}
		// アタッチしたアニメーションの総再生時間を取得する
		if(_attach_index != -1)
		{
			_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);
		}
		// 再生時間を初期化
		_play_time = 0.0f;
		// 再生時間をランダムにずらす
		switch(_status)
		{
			case STATUS::WAIT:
			{
				_play_time += rand() % 30; // 0 ～ 29 の揺らぎ
				break;
			}
		}
	}

	// 再生時間がアニメーションの総再生時間に達したら再生時間を0に戻す
	if(_play_time >= _total_time)
	{
		_play_time = 0.0f;
	}

	return true;
}

// 索敵範囲の設定
void Enemy::SetDetectionSector(float radius, float angle)
{
	_detection_sector.radius = radius;
	_detection_sector.angle = angle;
	_has_detection_sector = true;
}

// プレイヤーが索敵範囲内にいるかチェック
bool Enemy::IsPlayerInDetectionRange(const VECTOR& playerPos) const
{
	if (!_has_detection_sector)
	{
		return false;
	}

	// 索敵範囲情報を一時的な変数として作成
	DetectionSector currentSector;
	currentSector.center = _pos;
	currentSector.forward = _dir;
	currentSector.radius = _detection_sector.radius;
	currentSector.angle = _detection_sector.angle;

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
	if (!_has_detection_sector)
	{
		return;
	}

	// 索敵範囲の色（より目立つ色に変更）
	unsigned int color = GetColor(255, 0, 0); // 純粋な赤色

	// 扇形を描画するための分割数
	const int segments = 32;
	const float halfAngle = _detection_sector.angle * 0.5f;

	// 現在の敵の位置と向きを使用
	VECTOR center = _pos;
	VECTOR forward = _dir;

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

		VECTOR pos1 = VAdd(center, VGet(sinf(angle1) * _detection_sector.radius, 0, cosf(angle1) * _detection_sector.radius));
		VECTOR pos2 = VAdd(center, VGet(sinf(angle2) * _detection_sector.radius, 0, cosf(angle2) * _detection_sector.radius));

		// 3D空間での線描画
		DrawLine3D(pos1, pos2, color);

		// 少し上の位置にも線を描画して見えやすくする
		VECTOR pos1_up = VAdd(pos1, VGet(0, 10.0f, 0));
		VECTOR pos2_up = VAdd(pos2, VGet(0, 10.0f, 0));
		DrawLine3D(pos1_up, pos2_up, color);
	}

	// 中心から両端への線を描画
	VECTOR leftEdge = VAdd(center, VGet(sinf(startAngle) * _detection_sector.radius, 0, cosf(startAngle) * _detection_sector.radius));
	VECTOR rightEdge = VAdd(center, VGet(sinf(endAngle) * _detection_sector.radius, 0, cosf(endAngle) * _detection_sector.radius));

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
	MV1SetAttachAnimTime(_handle, _attach_index, _play_time);

	// 位置
	MV1SetPosition(_handle, _pos);
	// 向きからY軸回転を算出
	VECTOR vrot = { 0,0,0, };
	vrot.y = atan2f(-_dir.x, -_dir.z);
	MV1SetRotationXYZ(_handle, vrot);

	// 描画
	MV1DrawModel(_handle);

	// 索敵範囲の描画（デバッグ用）
	RenderDetectionSector();
	return true;
}