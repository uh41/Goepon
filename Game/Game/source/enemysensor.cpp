#include "enemysensor.h"
#include "appframe.h"
#include "map.h"

// 初期化
bool EnemySensor::Initialize()
{
	base::Initialize();

	// 索敵システムの初期化
	_bHasDetectionSector = false;	// 索敵範囲未設定
	_bSensorEnabled = true;			// センサー有効

	// 検出情報の初期化
	_detectionInfo.isDetected = false;	// 未検出
	_detectionInfo.timer = 0.0f;		// タイマー初期化
	_detectionInfo.detectorIndex = -1;	// 検出者インデックス初期化
	_detectionInfo.detectorPos = vec3::VGet(0.0f, 0.0f, 0.0f);	// 検出者位置初期化

	// 追跡情報の初期化
	_detectionInfo.isChasing = false;	// 追跡中フラグ初期化
	_detectionInfo.lastKnownPlayerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	// 最後に確認された位置初期化
	_detectionInfo.chaseTimer = 0.0f;	// 追跡タイマー初期化

	// デフォルトの索敵範囲設定
	SetDetectionSector(400.0f, 90.0f);//半径、角度



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
	// センサーが無効またはプレイヤーが存在しない場合は検出しない
	if (!player || !_bSensorEnabled)
	{
		return false;
	}

	vec::Vec3 playerPos = player->GetPos();	// プレイヤーの位置取得
	bool detected = IsPlayerInDetectionRange(playerPos);	// 索敵範囲内かチェック

	// 検出状態の更新
	if (detected)
	{
		// プレイヤーが範囲内にいる場合
		if (!_detectionInfo.isDetected)
		{
			// 新しく検出された場合
			_detectionInfo.isDetected = true;				// 検出フラグセット
			_detectionInfo.timer = DETECTION_DISPLAY_TIME;	// タイマーリセット
			_detectionInfo.detectorPos = _vPos;				// 検出者位置更新
		}

		// プレイヤーを検出中は常に位置を更新し、追跡タイマーをリセット
		_detectionInfo.lastKnownPlayerPos = playerPos;	// 最後に確認された位置更新
		_detectionInfo.isChasing = true;				// 追跡中フラグセット
		_detectionInfo.chaseTimer = CHASE_TIME;			// 追跡タイマーリセット
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
	_detectionInfo.isDetected = false;	// 未検出
	_detectionInfo.timer = 0.0f;		// タイマーリセット
	_detectionInfo.detectorIndex = -1;	// 検出者インデックスリセット
	_detectionInfo.detectorPos = vec3::VGet(0.0f, 0.0f, 0.0f);	// 検出者位置リセット

	// 追跡状態リセット
	_detectionInfo.isChasing = false;	// 追跡中フラグリセット
	_detectionInfo.lastKnownPlayerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	// 最後に確認された位置リセット
	_detectionInfo.chaseTimer = 0.0f;	// 追跡タイマーリセット
}

// 検出タイマーの更新
void EnemySensor::UpdateDetectionTimer()
{
	// 追跡タイマーの更新
	if (_detectionInfo.isChasing)
	{
		_detectionInfo.chaseTimer -= 1.0f / 60.0f; // 60FPSとして計算

		if (_detectionInfo.chaseTimer <= 0.0f)
		{
			_detectionInfo.isChasing = false;
			_detectionInfo.chaseTimer = 0.0f;
		}
	}

	// 検出表示タイマーの更新
	if (_detectionInfo.timer > 0.0f)
	{
		_detectionInfo.timer -= 1.0f / 60.0f; // 60FPSとして計算

		if (_detectionInfo.timer <= 0.0f)
		{
			_detectionInfo.isDetected = false;
			_detectionInfo.detectorIndex = -1;
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
vec::Vec3 EnemySensor::GetDetectionCenter() const
{
	// 敵の正面方向に一定距離進んだ位置を中心にする
	const float offsetDistance = _detectionSector.radius * 0.1f; // 半径の10%前方
	vec::Vec3 forwardNorm = vec3::VNorm(_vDir);
	return vec3::VAdd(_vPos, vec3::VScale(forwardNorm, offsetDistance));
}

// プレイヤーが索敵範囲内にいるかチェック
bool EnemySensor::IsPlayerInDetectionRange(const vec::Vec3& playerPos) const
{
	if (!_bHasDetectionSector || !_bSensorEnabled)
	{
		return false;
	}

	// 索敵範囲の中心位置を取得
	vec::Vec3 detectionCenter = GetDetectionCenter();

	// 索敵範囲の中心からプレイヤーへのベクトル
	vec::Vec3 toPlayer = vec3::VSub(playerPos, detectionCenter);

	// 距離チェック
	float distance = vec3::VSize(toPlayer);
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
	vec::Vec3 toPlayerNorm = vec3::VNorm(toPlayer);
	vec::Vec3 forwardNorm = vec3::VNorm(_vDir);

	// 正面方向との内積を計算
	float dot = vec::Vec3::Dot(forwardNorm, toPlayerNorm);

	// 内積から角度を計算（ラジアン）
	float angleRad = acosf(fmaxf(-1.0f, fminf(1.0f, dot)));

	// 度に変換
	float angleDeg = angleRad * 180.0f / DX_PI_F;

	// 扇形の半角と比較
	float halfAngle = _detectionSector.angle * 0.5f;

	// 角度チェック
	if (angleDeg > halfAngle)
	{
		return false; // 角度範囲外
	}

	//視線チェック - 敵の位置からプレイヤーの位置まで床の存在を一定間隔でチェック
	if (!CheckLineOfSight(detectionCenter, playerPos))
	{
		return false; // 視線が遮断されている
	}

	return true;
}

// 視線チェック - 指定した2点間で床なしの地点があるかチェック
bool EnemySensor::CheckLineOfSight(const vec::Vec3& startPos, const vec::Vec3& endPos) const
{
	// マップが設定されていない場合は視線が通るものとして処理
	if (!_map)
	{
		return true;
	}

	// チェック間隔（単位：ワールド座標）
	const float checkInterval = 30.0f;

	// 開始点から終了点へのベクトル
	vec::Vec3 direction = vec3::VSub(endPos, startPos);
	float totalDistance = vec3::VSize(direction);

	// 距離が短すぎる場合は視線が通るとみなす
	if (totalDistance < checkInterval)
	{
		return CheckFloorExistence(endPos);
	}

	// 正規化された方向ベクトル
	vec::Vec3 dirNorm = vec3::VNorm(direction);

	// チェック回数を計算
	int checkCount = static_cast<int>(totalDistance / checkInterval);

	// 一定間隔で床の存在をチェック
	for (int i = 1; i <= checkCount; i++)
	{
		float currentDistance = checkInterval * i;
		vec::Vec3 checkPos = vec3::VAdd(startPos, vec3::VScale(dirNorm, currentDistance));

		// この地点で床がない場合は視線が遮断される
		if (!CheckFloorExistence(checkPos))
		{
			return false;
		}
	}

	// 最終的に終了点の床もチェック
	return CheckFloorExistence(endPos);
}

bool EnemySensor::CheckFloorExistence(const vec::Vec3& position) const
{
	// マップが設定されていない場合は床があるものとして処理
	if (!_map)
	{
		return true;
	}

	vec::Vec3 startPos = vec3::VAdd(position, vec3::VGet(0.0f, 100.0f, 0.0f));
	vec::Vec3 endPos = vec3::VAdd(position, vec3::VGet(0.0f, -9999.0f, 0.0f));

	// _iFrameMapCollisionを取得して使用する例
	int frameMapCollision = _map->GetFrameMapCollision();

	// MAP_SELECT == 2 相当：ブロック毎にコリジョンチェック
	float nearestDist = FLT_MAX;
	bool hit = false;

	// ブロック毎にコリジョンチェック
	for (const auto& block : _map->GetBlockPosList())
	{
		// モデルハンドルが無効な場合はスキップ
		if (block.modelHandle < 0)
		{
			continue;
		}

		// map.cpp では Collision_01 を使って SetupCollInfo 済み
		const int frame = MV1SearchFrame(block.modelHandle, "Collision_01");
		if (frame < 0)
		{
			continue;
		}

		// コリジョンチェック
		MV1_COLL_RESULT_POLY poly = DxlibConverter::MV1CollCheckLine(
			block.modelHandle,
			frame,
			startPos,
			endPos
		);

		// 衝突している場合は最も近い衝突位置を記録
		if (poly.HitFlag == TRUE)
		{
			const vec::Vec3 hitPos = DxlibConverter::DxLibToVec(poly.HitPosition);
			const float dist = vec3::VSize(vec3::VSub(hitPos, startPos));

			// 最も近い衝突位置の更新
			if (dist < nearestDist)
			{
				nearestDist = dist;
				hit = true;
			}
		}
	}

	return hit;
}

// 床とのY軸方向の衝突位置を取得する関数
bool EnemySensor::GetFloorYCollision(const vec::Vec3& position, float colSubY, float& outY) const
{
	if(!_map)
	{
		return false;
	}

	vec::Vec3 startPos = vec3::VAdd(position, vec3::VGet(0.0f, 50.0f, 0.0f));	// 少し上からチェック
	vec::Vec3 endPos = vec3::VAdd(position, vec3::VGet(0.0f, -9999.0f, 0.0f));	// 大きく下までチェック

	float nearestDist = FLT_MAX;	// 最も近い衝突距離
	bool hit = false;				// 衝突フラグ
	float bestY = 0.0f;				// 最も近い衝突位置のY座標

	// MAP_SELECT == 2 相当：ブロック毎にコリジョンチェック
	for(const auto& block : _map->GetBlockPosList())
	{
		// モデルハンドルが無効な場合はスキップ
		if(block.modelHandle < 0)
		{
			continue;
		}

		// map.cpp では Collision_01 を使って SetupCollInfo 済み
		const int frame = MV1SearchFrame(block.modelHandle, "Collision_01");
		if(frame < 0)
		{
			continue;
		}

		// コリジョンチェック
		MV1_COLL_RESULT_POLY poly = DxlibConverter::MV1CollCheckLine(
			block.modelHandle,
			frame,
			startPos,
			endPos
		);

		// 衝突している場合は最も近い衝突位置を記録
		if(poly.HitFlag == TRUE)
		{
			const vec::Vec3 hitPos = DxlibConverter::DxLibToVec(poly.HitPosition);	// 衝突位置取得
			const float dist = vec3::VSize(vec3::VSub(hitPos, startPos));			// 衝突位置までの距離計算

			// 最も近い衝突位置の更新
			if(dist < nearestDist)
			{
				nearestDist = dist;	// 最短距離更新
				bestY = hitPos.y;	// 衝突位置のY座標更新
				hit = true;			// 衝突フラグセット
			}
		}
	}

	// 衝突していればY座標を出力
	if(hit)
	{
		outY = bestY;
		return true;
	}

	return false;
}

// デバッグ用：索敵範囲の描画
void EnemySensor::RenderDetectionSector() const
{
	// 索敵範囲が設定されていない場合は描画しない
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
	vec::Vec3 center = GetDetectionCenter();
	vec::Vec3 forward = vec3::VNorm(_vDir);

	// Y軸回りの回転を使って扇形を描画
	// 敵の向いている方向を基準角度として計算
	float baseAngle = atan2f(forward.x, forward.z);

	// 扇形の輪郭線を描画
	for (int i = 0; i < segments; i++)
	{
		float angle1 = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * i / (float)segments));
		float angle2 = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * (i + 1) / (float)segments));

		// 極座標から直交座標への変換
		vec::Vec3 pos1 = vec3::VAdd(center, vec3::VGet(sinf(angle1) * _detectionSector.radius, 0.0f, cosf(angle1) * _detectionSector.radius));
		vec::Vec3 pos2 = vec3::VAdd(center, vec3::VGet(sinf(angle2) * _detectionSector.radius, 0.0f, cosf(angle2) * _detectionSector.radius));

		// 3D空間での線描画
		DxlibConverter::DrawLine3D(pos1, pos2, color);

		// 少し上の位置にも線を描画して見えやすくする
		vec::Vec3 pos1_up = vec3::VAdd(pos1, vec3::VGet(0.0f, 10.0f, 0.0f));
		vec::Vec3 pos2_up = vec3::VAdd(pos2, vec3::VGet(0.0f, 10.0f, 0.0f));
		DxlibConverter::DrawLine3D(pos1_up, pos2_up, color);
	}

	// 中心から両端への線を描画
	float leftAngle = baseAngle - halfAngleRad;
	float rightAngle = baseAngle + halfAngleRad;

	// 極座標から直交座標への変換
	vec::Vec3 leftEdge = vec3::VAdd(center, vec3::VGet(sinf(leftAngle) * _detectionSector.radius, 0.0f, cosf(leftAngle) * _detectionSector.radius));
	vec::Vec3 rightEdge = vec3::VAdd(center, vec3::VGet(sinf(rightAngle) * _detectionSector.radius, 0.0f, cosf(rightAngle) * _detectionSector.radius));

	// 3D空間での線描画
	DxlibConverter::DrawLine3D(center, leftEdge, color);
	DxlibConverter::DrawLine3D(center, rightEdge, color);

	// 少し上の位置にも線を描画
	vec::Vec3 center_up = vec3::VAdd(center, vec3::VGet(0.0f, 10.0f, 0.0f));
	vec::Vec3 leftEdge_up = vec3::VAdd(leftEdge, vec3::VGet(0.0f, 10.0f, 0.0f));
	vec::Vec3 rightEdge_up = vec3::VAdd(rightEdge, vec3::VGet(0.0f, 10.0f, 0.0f));

	// 3D空間での線描画
	DxlibConverter::DrawLine3D(center_up, leftEdge_up, color);
	DxlibConverter::DrawLine3D(center_up, rightEdge_up, color);

	// 敵の正面方向を示す緑の線を描画（敵の位置から索敵中心まで）
	vec::Vec3 enemyPos = _vPos;
	DxlibConverter::DrawLine3D(vec3::VAdd(enemyPos, vec3::VGet(0.0f, 5.0f, 0.0f)), vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, 0.0f)), GetColor(0, 255, 0));

	// 中心点を示すマーカーを描画
	vec::Vec3 marker1 = vec3::VAdd(center, vec3::VGet(-10.0f, 5.0f, 0.0f));
	vec::Vec3 marker2 = vec3::VAdd(center, vec3::VGet(10.0f, 5.0f, 0.0f));
	vec::Vec3 marker3 = vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, -10.0f));
	vec::Vec3 marker4 = vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, 10.0f));

	// 十字マーカーの描画
	DxlibConverter::DrawLine3D(marker1, marker2, color);
	DxlibConverter::DrawLine3D(marker3, marker4, color);
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