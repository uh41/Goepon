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

	{
		DrawFormatString(
			20,
			90,
			GetColor(255, 255, 255),
			"Enemy Pos: (%.2f, %.2f, %.2f)  Dir: (%.2f, %.2f, %.2f)",
			_vPos.x, _vPos.y, _vPos.z,
			_vDir.x, _vDir.y, _vDir.z
		);
	}

	// デバッグ表示：Collision_01(床)に乗っているか
	{
		float floorY = 0.0f;
		const float colSubY = 100.0f;
		const bool onCollision01 = GetFloorYCollision(_vPos, colSubY, floorY);

		const unsigned int color = onCollision01 ? GetColor(0, 255, 0) : GetColor(255, 0, 0);
		const float diffY = onCollision01 ? (_vPos.y - floorY) : 0.0f;

		DrawFormatString(
			20,
			120,
			color,
			"Enemy On Collision_01: %s  EnemyY: %.2f  FloorY: %.2f  DiffY: %.2f",
			onCollision01 ? "TRUE" : "FALSE",
			_vPos.y,
			floorY,
			diffY
		);
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
	
	// プレイヤーのカプセル情報を計算
	// PlayerBaseからコリジョン情報を取得
	float playerColSubY = player->GetColSubY();      // 腰の高さ
	float playerColRadius = player->GetCollisionR(); // コリジョン半径

	// カプセルの上端と下端を計算
	vec::Vec3 playerCapsuleTop = vec3::VAdd(playerPos, vec3::VGet(0.0f, playerColSubY, 0.0f));
	vec::Vec3 playerCapsuleBottom = vec3::VSub(playerPos, vec3::VGet(0.0f, playerColSubY * 0.5f, 0.0f));

	// カプセル判定版の索敵範囲チェック
	bool detected = IsPlayerInDetectionRangeWithCapsule(
		playerPos,
		playerCapsuleTop,
		playerCapsuleBottom,
		playerColRadius
	);

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
	else
	{
		// プレイヤーが範囲外だが、まだ追跡中の場合
		// 最後に確認された位置は更新しない（記憶している位置を維持）
	}

	return detected;
}

// カプセル判定版の索敵範囲チェック
bool EnemySensor::IsPlayerInDetectionRangeWithCapsule(
	const vec::Vec3& playerPos,
	const vec::Vec3& playerCapsuleTop,
	const vec::Vec3& playerCapsuleBottom,
	float playerCapsuleRadius) const
{
	if (!_bHasDetectionSector || !_bSensorEnabled)
	{
		return false;
	}

	// 索敵範囲の中心位置を取得
	vec::Vec3 detectionCenter = GetDetectionCenter();

	// 扇形の半角をラジアンに変換
	float halfAngleRad = (_detectionSector.angle * 0.5f) * DX_PI_F / 180.0f;

	// 正規化された前方ベクトル
	vec::Vec3 forwardNorm = vec3::VNorm(_vDir);

	// CollisionManagerのCheckSectorToCapsuleを使用
	bool detected = CollisionManager::GetInstance()->CheckSectorToCapsule(
		detectionCenter,
		forwardNorm,
		_detectionSector.radius,
		halfAngleRad,
		playerCapsuleTop,
		playerCapsuleBottom,
		playerCapsuleRadius
	);

	if (!detected)
	{
		return false;
	}

	// 視線チェック - 敵の位置からプレイヤーの位置まで床の存在を一定間隔でチェック
	if (!CheckLineOfSight(detectionCenter, playerPos))
	{
		return false; // 視線が遮断されている
	}

	return true;
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

	// 角度が半角を超えている場合は範囲外
	if (angleDeg > halfAngle)
	{
		return false; // 角度範囲外
	}

	// 視線チェック - 敵の位置からプレイヤーの位置まで床の存在を一定間隔でチェック
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
	int checkCount = StCas<int>(totalDistance / checkInterval);

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

// 床の存在を確認する関数
bool EnemySensor::CheckFloorExistence(const vec::Vec3& position) const
{
	// マップが設定されていない場合は床があるものとして処理
	if(!_map)
	{
		return true;
	}

	// 「乗れるか」判定なので、真下へのレイ開始高さは最低限必要
	// （position.y が床より下の場合でも拾えるように余裕を持たせる）
	constexpr float kColSubY = 100.0f;

	vec::Vec3 hitPos{};
	for(const auto& block : _map->GetBlockPosList())
	{
		if(block.modelHandle < 0)
		{
			continue;
		}

		//const int frame = block.collisionFrame;
		//if(frame < 0)
		//{
		//	continue;
		//}

		if(CollisionManager::GetInstance()->CheckPositionToMV1Collision(
			position,
			block.modelHandle,
			block.collisionFrame,
			kColSubY,
			hitPos))
		{
			return true;
		}
	}

	return false;
}

// Collision_01 にヒットした床Yを返す（最も近い床を採用）
bool EnemySensor::GetFloorYCollision(const vec::Vec3& position, float colSubY, float& outY) const
{
	if(!_map)
	{
		return false;
	}

	bool hit = false;
	float bestY = 0.0f;
	float nearestDist = FLT_MAX;

	const vec::Vec3 startPos = vec3::VAdd(position, vec3::VGet(0.0f, colSubY, 0.0f));

	vec::Vec3 hitPos{};
	for(const auto& block : _map->GetBlockPosList())
	{
		if(block.modelHandle < 0)
		{
			continue;
		}

		// 「ブロック全体」ではなく、「Collision_01 フレーム(床)」のみを対象にする
		const int frame = block.collisionFrame;
		if(frame < 0)
		{
			continue;
		}

		// 床との当たり判定チェック
		if(!CollisionManager::GetInstance()->CheckPositionToMV1Collision(
			position,
			block.modelHandle,
			block.collisionFrame,
			colSubY,
			hitPos))
		{
			continue;
		}

		// 開始点からの距離が最小のものを採用（最上面/直下の床を取りたい）
		const float dist = vec3::VSize(vec3::VSub(hitPos, startPos));
		if(dist < nearestDist)
		{
			nearestDist = dist;
			bestY = hitPos.y;
			hit = true;
		}
	}

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
	const int segments = 16;
	const float halfAngleRad = (_detectionSector.angle * 0.5f) * DX_PI_F / 180.0f;

	// 索敵範囲の中心位置を取得
	vec::Vec3 center = GetDetectionCenter();
	vec::Vec3 forward = vec3::VNorm(_vDir);

	// Y軸回りの回転を使って扇形を描画
	// 敵の向いている方向を基準角度として計算
	float baseAngle = atan2f(forward.x, forward.z);

	// 半径方向の分割数とステップ
	const int radialSegments = 5;
	const float radialStep = _detectionSector.radius / radialSegments;

	// 高さオフセットを定数化（毎回計算しない）
	const vec::Vec3 heightOffset = vec3::VGet(0.0f, 10.0f, 0.0f);

	// 扇形の輪郭線を描画
	for (int i = 0; i < segments; i++)
	{
		float angle1 = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * i / StCas<float>(segments)));
		float angle2 = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * (i + 1) / StCas<float>(segments)));

		// 事前計算：三角関数の結果をキャッシュ
		float sin1 = sinf(angle1);
		float cos1 = cosf(angle1);
		float sin2 = sinf(angle2);
		float cos2 = cosf(angle2);
		
		// 半径方向に分割してチェック
		vec::Vec3 prevPos1, prevPos2;
		bool hasPrevPos = false;

		for (int r = 1; r <= radialSegments; r++)
		{
			float currentRadius = radialStep * r;

			// キャッシュした三角関数の値を使用
			vec::Vec3 pos1 = vec3::VAdd(center, vec3::VGet(sin1 * currentRadius, 0.0f, cos1 * currentRadius));
			vec::Vec3 pos2 = vec3::VAdd(center, vec3::VGet(sin2 * currentRadius, 0.0f, cos2 * currentRadius));

			// 視線チェックを含めた検出可能判定
			bool canDetect1 = CheckFloorExistence(pos1) && CheckLineOfSight(center, pos1);
			bool canDetect2 = CheckFloorExistence(pos2) && CheckLineOfSight(center, pos2);

			// 両方の点が検出可能な場合のみ線を描画
			if (canDetect1 && canDetect2)
			{
				// 円弧方向の線
				DxlibConverter::DrawLine3D(pos1, pos2, color);

				// 少し上の位置にも線を描画して見えやすくする
				vec::Vec3 pos1_up = vec3::VAdd(pos1, heightOffset);
				vec::Vec3 pos2_up = vec3::VAdd(pos2, heightOffset);
				DxlibConverter::DrawLine3D(pos1_up, pos2_up, color);

				// 前の半径位置から現在の半径位置への線（放射状の線）
				if (hasPrevPos)
				{
					DxlibConverter::DrawLine3D(prevPos1, pos1, color);
					DxlibConverter::DrawLine3D(prevPos2, pos2, color);
					vec::Vec3 prevPos1_up = vec3::VAdd(prevPos1, heightOffset);
					vec::Vec3 prevPos2_up = vec3::VAdd(prevPos2, heightOffset);
					DxlibConverter::DrawLine3D(prevPos1_up, pos1_up, color);
					DxlibConverter::DrawLine3D(prevPos2_up, pos2_up, color);
				}

				prevPos1 = pos1;
				prevPos2 = pos2;
				hasPrevPos = true;
			}
			else
			{
				// 床がない場合は前の位置情報をリセット
				hasPrevPos = false;
			}
		}
	}

	// 中心から両端への線を描画
	float leftAngle = baseAngle - halfAngleRad;
	float rightAngle = baseAngle + halfAngleRad;

	// 三角関数の事前計算
	float sinLeft = sinf(leftAngle);
	float cosLeft = cosf(leftAngle);
	float sinRight = sinf(rightAngle);
	float cosRight = cosf(rightAngle);

	// 各エッジラインを分割してチェック
	for (int r = 1; r <= radialSegments; r++)
	{
		float currentRadius1 = radialStep * (r - 1);
		float currentRadius2 = radialStep * r;

		// キャッシュした三角関数の値を使用
		vec::Vec3 leftEdge1 = vec3::VAdd(center, vec3::VGet(sinLeft * currentRadius1, 0.0f, cosLeft * currentRadius1));
		vec::Vec3 leftEdge2 = vec3::VAdd(center, vec3::VGet(sinLeft * currentRadius2, 0.0f, cosLeft * currentRadius2));
		vec::Vec3 rightEdge1 = vec3::VAdd(center, vec3::VGet(sinRight * currentRadius1, 0.0f, cosRight * currentRadius1));
		vec::Vec3 rightEdge2 = vec3::VAdd(center, vec3::VGet(sinRight * currentRadius2, 0.0f, cosRight * currentRadius2));

		// 視線チェックを含めた検出可能判定
		bool canDetectLeft1 = CheckFloorExistence(leftEdge1) && CheckLineOfSight(center, leftEdge1);
		bool canDetectLeft2 = CheckFloorExistence(leftEdge2) && CheckLineOfSight(center, leftEdge2);
		bool canDetectRight1 = CheckFloorExistence(rightEdge1) && CheckLineOfSight(center, rightEdge1);
		bool canDetectRight2 = CheckFloorExistence(rightEdge2) && CheckLineOfSight(center, rightEdge2);

		// 左端の線（両方の点が検出可能な場合のみ）
		if (canDetectLeft1 && canDetectLeft2)
		{
			DxlibConverter::DrawLine3D(leftEdge1, leftEdge2, color);
			vec::Vec3 leftEdge1_up = vec3::VAdd(leftEdge1, heightOffset);
			vec::Vec3 leftEdge2_up = vec3::VAdd(leftEdge2, heightOffset);
			DxlibConverter::DrawLine3D(leftEdge1_up, leftEdge2_up, color);
		}

		// 右端の線（両方の点が検出可能な場合のみ）
		if (canDetectRight1 && canDetectRight2)
		{
			DxlibConverter::DrawLine3D(rightEdge1, rightEdge2, color);
			vec::Vec3 rightEdge1_up = vec3::VAdd(rightEdge1, heightOffset);
			vec::Vec3 rightEdge2_up = vec3::VAdd(rightEdge2, heightOffset);
			DxlibConverter::DrawLine3D(rightEdge1_up, rightEdge2_up, color);
		}
	}
	
	// 敵の正面方向を示す緑の線を描画（敵の位置から索敵中心まで）
	const vec::Vec3 lineOffset = vec3::VGet(0.0f, 5.0f, 0.0f);
	DxlibConverter::DrawLine3D(
		vec3::VAdd(_vPos, lineOffset),
		vec3::VAdd(center, lineOffset),
		GetColor(0, 255, 0)
	);

	// 中心点を示すマーカーを描画
	const float markerSize = 10.0f;
	vec::Vec3 centerMarker = vec3::VAdd(center, lineOffset);
	vec::Vec3 marker1 = vec3::VAdd(centerMarker, vec3::VGet(-markerSize, 0.0f, 0.0f));
	vec::Vec3 marker2 = vec3::VAdd(centerMarker, vec3::VGet(markerSize, 0.0f, 0.0f));
	vec::Vec3 marker3 = vec3::VAdd(centerMarker, vec3::VGet(0.0f, 0.0f, -markerSize));
	vec::Vec3 marker4 = vec3::VAdd(centerMarker, vec3::VGet(0.0f, 0.0f, markerSize));

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
		int textWidth = GetDrawStringWidth(foundText, StCas<int>(strlen(foundText)));
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