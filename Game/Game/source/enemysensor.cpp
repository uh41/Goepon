#include "enemysensor.h"
#include "appframe.h"

// 初期化
bool EnemySensor::Initialize()
{
	base::Initialize();

	// 索敵システムの初期化
	_bHasDetectionSector = false;
	_bSensorEnabled = true;

	// 検出情報の初期化
	_detectionInfo.isDetected = false;
	_detectionInfo.timer = 0.0f;
	_detectionInfo.detectorIndex = -1;
	_detectionInfo.detectorPos = VGet(0.0f, 0.0f, 0.0f);

	// 追跡情報の初期化
	_detectionInfo.isChasing = false;
	_detectionInfo.lastKnownPlayerPos = VGet(0.0f, 0.0f, 0.0f);
	_detectionInfo.chaseTimer = 0.0f;

	// デフォルトの索敵範囲設定
	SetDetectionSector(400.0f, 60.0f);

	return true;
}

// 終了
bool EnemySensor::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool EnemySensor::Process()
{
	base::Process();

	// センサーが無効の場合は処理しない
	if (!_bSensorEnabled)
	{
		return true;
	}

	// 検出タイマーの更新
	UpdateDetectionTimer();

	return true;
}

// 描画処理
bool EnemySensor::Render()
{
	base::Render();

	// デバッグ表示：索敵範囲の描画
	if (_bSensorEnabled)
	{
		RenderDetectionSector();
	}

	return true;
}

// プレイヤーの検出チェック
bool EnemySensor::CheckPlayerDetection(PlayerBase* player)
{
	if (!player || !_bSensorEnabled)
	{
		return false;
	}

	VECTOR playerPos = player->GetPos();
	bool detected = IsPlayerInDetectionRange(playerPos);

	if (detected)
	{
		// プレイヤーが範囲内にいる場合
		if (!_detectionInfo.isDetected)
		{
			// 新しく検出された場合
			_detectionInfo.isDetected = true;
			_detectionInfo.timer = DETECTION_DISPLAY_TIME;
			_detectionInfo.detectorPos = _vPos;
		}

		// プレイヤーを検出中は常に位置を更新し、追跡タイマーをリセット
		_detectionInfo.lastKnownPlayerPos = playerPos;
		_detectionInfo.isChasing = true;
		_detectionInfo.chaseTimer = CHASE_TIME;
	}
	else if (_detectionInfo.isChasing)
	{
		// プレイヤーが範囲外だが、まだ追跡中の場合
		// 最後に確認された位置は更新しない（記憶している位置を維持）
	}

	return detected;
}

// 検出状態のリセット
void EnemySensor::ResetDetection()
{
	_detectionInfo.isDetected = false;
	_detectionInfo.timer = 0.0f;
	_detectionInfo.detectorIndex = -1;
	_detectionInfo.detectorPos = VGet(0.0f, 0.0f, 0.0f);

	// 追跡状態リセット
	_detectionInfo.isChasing = false;
	_detectionInfo.lastKnownPlayerPos = VGet(0.0f, 0.0f, 0.0f);
	_detectionInfo.chaseTimer = 0.0f;
}

// 検出タイマーの更新
void EnemySensor::UpdateDetectionTimer()
{
	if (_detectionInfo.timer > 0.0f)
	{
		_detectionInfo.timer -= 1.0f / 60.0f; // 60FPSとして計算

		if (_detectionInfo.timer <= 0.0f)
		{
			_detectionInfo.isDetected = false;
			_detectionInfo.detectorIndex = -1;
			// 追跡タイマーが切れたら追跡を停止
			_detectionInfo.isChasing = false;
		}
	}
}

// 索敵範囲の設定
void EnemySensor::SetDetectionSector(float radius, float angle)
{
	_detectionSector.radius = radius;
	_detectionSector.angle = angle;
	_bHasDetectionSector = true;
}

// 索敵範囲の中心位置を取得（敵の正面に配置）
VECTOR EnemySensor::GetDetectionCenter() const
{
	// 敵の正面方向に一定距離進んだ位置を中心にする
	const float offsetDistance = _detectionSector.radius * 0.3f; // 半径の30%前方
	VECTOR forwardNorm = VNorm(_vDir);
	return VAdd(_vPos, VScale(forwardNorm, offsetDistance));
}

// プレイヤーが索敵範囲内にいるかチェック
bool EnemySensor::IsPlayerInDetectionRange(const VECTOR& playerPos) const
{
	if (!_bHasDetectionSector || !_bSensorEnabled)
	{
		return false;
	}

	// 索敵範囲の中心位置を取得
	VECTOR detectionCenter = GetDetectionCenter();

	// 索敵範囲の中心からプレイヤーへのベクトル
	VECTOR toPlayer = VSub(playerPos, detectionCenter);

	// 距離チェック
	float distance = VSize(toPlayer);
	if (distance > _detectionSector.radius)
	{
		return false; // 範囲外
	}

	// 距離が0に近い場合（同じ位置）は範囲内とみなす
	if (distance < 0.001f)
	{
		return true;
	}

	// 角度チェック - 正規化したベクトルで内積計算
	VECTOR toPlayerNorm = VNorm(toPlayer);
	VECTOR forwardNorm = VNorm(_vDir);

	// 正面方向との内積を計算
	float dot = VDot(forwardNorm, toPlayerNorm);

	// 内積から角度を計算（ラジアン）
	float angleRad = acosf(fmaxf(-1.0f, fminf(1.0f, dot)));

	// 度に変換
	float angleDeg = angleRad * 180.0f / DX_PI_F;

	// 扇形の半角と比較
	float halfAngle = _detectionSector.angle * 0.5f;

	return angleDeg <= halfAngle;
}

// デバッグ用：索敵範囲の描画
void EnemySensor::RenderDetectionSector() const
{
	if (!_bHasDetectionSector)
	{
		return;
	}

	// 索敵範囲の色
	unsigned int color = _detectionInfo.isDetected ? GetColor(255, 255, 0) : GetColor(255, 0, 0);

	// 扇形を描画するための分割数
	const int segments = 32;
	const float halfAngleRad = (_detectionSector.angle * 0.5f) * DX_PI_F / 180.0f;

	// 索敵範囲の中心位置を取得
	VECTOR center = GetDetectionCenter();
	VECTOR forward = VNorm(_vDir);

	// Y軸回りの回転を使って扇形を描画
	// 敵の向いている方向を基準角度として計算
	float baseAngle = atan2f(forward.x, forward.z);

	// 扇形の輪郭線を描画
	for (int i = 0; i < segments; i++)
	{
		float angle1 = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * i / (float)segments));
		float angle2 = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * (i + 1) / (float)segments));

		// 極座標から直交座標への変換
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
	float leftAngle = baseAngle - halfAngleRad;
	float rightAngle = baseAngle + halfAngleRad;

	VECTOR leftEdge = VAdd(center, VGet(sinf(leftAngle) * _detectionSector.radius, 0, cosf(leftAngle) * _detectionSector.radius));
	VECTOR rightEdge = VAdd(center, VGet(sinf(rightAngle) * _detectionSector.radius, 0, cosf(rightAngle) * _detectionSector.radius));

	DrawLine3D(center, leftEdge, color);
	DrawLine3D(center, rightEdge, color);

	// 少し上の位置にも線を描画
	VECTOR center_up = VAdd(center, VGet(0, 10.0f, 0));
	VECTOR leftEdge_up = VAdd(leftEdge, VGet(0, 10.0f, 0));
	VECTOR rightEdge_up = VAdd(rightEdge, VGet(0, 10.0f, 0));

	DrawLine3D(center_up, leftEdge_up, color);
	DrawLine3D(center_up, rightEdge_up, color);

	// 敵の正面方向を示す緑の線を描画（敵の位置から索敵中心まで）
	VECTOR enemyPos = _vPos;
	DrawLine3D(VAdd(enemyPos, VGet(0, 5.0f, 0)), VAdd(center, VGet(0, 5.0f, 0)), GetColor(0, 255, 0));

	// 中心点を示すマーカーを描画
	VECTOR marker1 = VAdd(center, VGet(-10.0f, 5.0f, 0));
	VECTOR marker2 = VAdd(center, VGet(10.0f, 5.0f, 0));
	VECTOR marker3 = VAdd(center, VGet(0, 5.0f, -10.0f));
	VECTOR marker4 = VAdd(center, VGet(0, 5.0f, 10.0f));

	DrawLine3D(marker1, marker2, color);
	DrawLine3D(marker3, marker4, color);
}

// 検出UI表示
void EnemySensor::RenderDetectionUI() const
{
	if (_detectionInfo.isDetected && _detectionInfo.timer > 0.0f)
	{
		// 画面中央に大きく「found」を表示
		int screenWidth = 1920;  // 画面幅
		int screenHeight = 1080; // 画面高さ

		// フォントサイズを大きく設定
		SetFontSize(64);

		// 「found」の文字列の幅を取得して中央揃え
		const char* foundText = "found";
		int textWidth = GetDrawStringWidth(foundText, static_cast<int>(strlen(foundText)));
		int x = (screenWidth - textWidth) / 2;
		int y = screenHeight / 2 - 32;

		// 背景色で縁取り
		DrawString(x - 2, y - 2, foundText, GetColor(0, 0, 0));
		DrawString(x + 2, y - 2, foundText, GetColor(0, 0, 0));
		DrawString(x - 2, y + 2, foundText, GetColor(0, 0, 0));
		DrawString(x + 2, y + 2, foundText, GetColor(0, 0, 0));

		// メイン文字（赤色）
		DrawString(x, y, foundText, GetColor(255, 0, 0));

		// フォントサイズを元に戻す
		SetFontSize(16);

		// タイマー情報
		DrawFormatString(x, y + 80, GetColor(255, 255, 0),
			"Timer: %.1f", _detectionInfo.timer);
	}
}