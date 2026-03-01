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

	// 追跡情報の初期化
	_detectionInfo.isChasing = false;	// 追跡中フラグ初期化
	_detectionInfo.lastKnownPlayerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	// 最後に確認された位置初期化
	_detectionInfo.chaseTimer = 0.0f;	// 追跡タイマー初期化

	_detectionFrameCount = 0;
	_CanDetectionResult = false;

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

	// フレーム間隔(扇形判定の負荷軽減用)
	_detectionFrameCount++;
	if (_detectionFrameCount < DetectionFrame)
	{
		return _CanDetectionResult;
	}
	_detectionFrameCount = 0;

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
		}

		// プレイヤーを検出中は常に位置を更新し、追跡タイマーをリセット
		_detectionInfo.lastKnownPlayerPos = playerPos;	// 最後に確認された位置更新
		_detectionInfo.isChasing = true;				// 追跡中フラグセット
		_detectionInfo.chaseTimer = CHASE_TIME;			// 追跡タイマーリセット
	}

	_CanDetectionResult = detected;

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

	// 追跡状態リセット
	_detectionInfo.isChasing = false;	// 追跡中フラグリセット
	_detectionInfo.lastKnownPlayerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	// 最後に確認された位置リセット
	_detectionInfo.chaseTimer = 0.0f;	// 追跡タイマーリセット

	_detectionFrameCount = 0;
	_CanDetectionResult = false;
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
	// 半透明の塗りつぶし色
	unsigned int fillColor = _detectionInfo.isDetected ? GetColor(255, 255, 0) : GetColor(255, 0, 0);

	// 扇形を描画するための分割数
	const int angleSegments = 32; // 角度方向の分割数
	const int radiusSegments = 10; // 半径方向の分割数
	const float halfAngleRad = (_detectionSector.angle * 0.5f) * DX_PI_F / 180.0f;

	// 索敵範囲の中心位置を取得
	vec::Vec3 center = GetDetectionCenter();
	vec::Vec3 forward = vec3::VNorm(_vDir);

	// Y軸回りの回転を使って扇形を描画
	// 敵の向いている方向を基準角度として計算
	float baseAngle = atan2f(forward.x, forward.z);

	// 高さオフセットを定数化（毎回計算しない）
	const vec::Vec3 heightOffset = vec3::VGet(0.0f, 10.0f, 0.0f);

	// 扇形を三角形ポリゴンで塗りつぶす（半径方向と円周方向の両方で分割）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); // 半透明に設定

	// 半径方向にループ
	for (int r = 0; r < radiusSegments; r++)
	{
		float innerRadius = (_detectionSector.radius / radiusSegments) * r;
		float outerRadius = (_detectionSector.radius / radiusSegments) * (r + 1);

		vec::Vec3 prevInnerPos, prevOuterPos;
		bool hasPrevPos = false;

		// 角度方向にループ
		for (int i = 0; i <= angleSegments; i++)
		{
			float angle = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * i / StCas<float>(angleSegments)));

			// 三角関数の結果をキャッシュ
			float sinAngle = sinf(angle);
			float cosAngle = cosf(angle);

			// 内側と外側の円周上の点を計算
			vec::Vec3 innerPos = vec3::VAdd(center, vec3::VGet(sinAngle * innerRadius, 0.0f, cosAngle * innerRadius));
			vec::Vec3 outerPos = vec3::VAdd(center, vec3::VGet(sinAngle * outerRadius, 0.0f, cosAngle * outerRadius));

			// 床の存在チェックと視線チェック（中心から外側の点まで）
			bool canDetect = CheckFloorExistence(outerPos) && CheckLineOfSight(center, outerPos);

			if (canDetect)
			{
				if (hasPrevPos)
				{
					// 4点で2つの三角形を描画（四角形を構成）
					VERTEX3D vertices[6]; // 2つの三角形 = 6頂点

					// 1つ目の三角形: prevInner, prevOuter, innerPos
					vertices[0].pos = VGet(prevInnerPos.x, prevInnerPos.y, prevInnerPos.z);
					vertices[0].norm = VGet(0.0f, 1.0f, 0.0f);
					
					vertices[0].spc = GetColorU8(0, 0, 0, 0);
					vertices[0].u = 0.0f;
					vertices[0].v = 0.0f;
					vertices[0].su = 0.0f;
					vertices[0].sv = 0.0f;

					vertices[1].pos = VGet(prevOuterPos.x, prevOuterPos.y, prevOuterPos.z);
					vertices[1].norm = VGet(0.0f, 1.0f, 0.0f);
					
					vertices[1].spc = GetColorU8(0, 0, 0, 0);
					vertices[1].u = 0.0f;
					vertices[1].v = 0.0f;
					vertices[1].su = 0.0f;
					vertices[1].sv = 0.0f;

					vertices[2].pos = VGet(innerPos.x, innerPos.y, innerPos.z);
					vertices[2].norm = VGet(0.0f, 1.0f, 0.0f);
					
					vertices[2].spc = GetColorU8(0, 0, 0, 0);
					vertices[2].u = 0.0f;
					vertices[2].v = 0.0f;
					vertices[2].su = 0.0f;
					vertices[2].sv = 0.0f;

					// 2つ目の三角形: prevOuter, outerPos, innerPos
					vertices[3].pos = VGet(prevOuterPos.x, prevOuterPos.y, prevOuterPos.z);
					vertices[3].norm = VGet(0.0f, 1.0f, 0.0f);
					
					vertices[3].spc = GetColorU8(0, 0, 0, 0);
					vertices[3].u = 0.0f;
					vertices[3].v = 0.0f;
					vertices[3].su = 0.0f;
					vertices[3].sv = 0.0f;

					vertices[4].pos = VGet(outerPos.x, outerPos.y, outerPos.z);
					vertices[4].norm = VGet(0.0f, 1.0f, 0.0f);
					
					vertices[4].spc = GetColorU8(0, 0, 0, 0);
					vertices[4].u = 0.0f;
					vertices[4].v = 0.0f;
					vertices[4].su = 0.0f;
					vertices[4].sv = 0.0f;

					vertices[5].pos = VGet(innerPos.x, innerPos.y, innerPos.z);
					vertices[5].norm = VGet(0.0f, 1.0f, 0.0f);
					
					vertices[5].spc = GetColorU8(0, 0, 0, 0);
					vertices[5].u = 0.0f;
					vertices[5].v = 0.0f;
					vertices[5].su = 0.0f;
					vertices[5].sv = 0.0f;

					// 2つの三角形を描画
					DrawPolygon3D(vertices, 2, DX_NONE_GRAPH, TRUE);

					// 少し上の位置にも描画して見えやすくする
					vec::Vec3 prevInnerPos_up = vec3::VAdd(prevInnerPos, heightOffset);
					vec::Vec3 prevOuterPos_up = vec3::VAdd(prevOuterPos, heightOffset);
					vec::Vec3 innerPos_up = vec3::VAdd(innerPos, heightOffset);
					vec::Vec3 outerPos_up = vec3::VAdd(outerPos, heightOffset);

					vertices[0].pos = VGet(prevInnerPos_up.x, prevInnerPos_up.y, prevInnerPos_up.z);
					vertices[1].pos = VGet(prevOuterPos_up.x, prevOuterPos_up.y, prevOuterPos_up.z);
					vertices[2].pos = VGet(innerPos_up.x, innerPos_up.y, innerPos_up.z);
					vertices[3].pos = VGet(prevOuterPos_up.x, prevOuterPos_up.y, prevOuterPos_up.z);
					vertices[4].pos = VGet(outerPos_up.x, outerPos_up.y, outerPos_up.z);
					vertices[5].pos = VGet(innerPos_up.x, innerPos_up.y, innerPos_up.z);

					DrawPolygon3D(vertices, 2, DX_NONE_GRAPH, TRUE);
				}

				prevInnerPos = innerPos;
				prevOuterPos = outerPos;
				hasPrevPos = true;
			}
			else
			{
				hasPrevPos = false;
			}
		}
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ブレンドモードを元に戻す

	// 外周の線を描画（輪郭を強調）
	vec::Vec3 prevPos;
	bool hasPrevPos = false;
	for (int i = 0; i <= angleSegments; i++)
	{
		float angle = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * i / StCas<float>(angleSegments)));

		float sinAngle = sinf(angle);
		float cosAngle = cosf(angle);

		vec::Vec3 pos = vec3::VAdd(center, vec3::VGet(sinAngle * _detectionSector.radius, 0.0f, cosAngle * _detectionSector.radius));

		// 床の存在チェックと視線チェック
		bool canDetect = CheckFloorExistence(pos) && CheckLineOfSight(center, pos);

		if (canDetect)
		{
			if (hasPrevPos)
			{
				DxlibConverter::DrawLine3D(prevPos, pos, color);

				vec::Vec3 prevPos_up = vec3::VAdd(prevPos, heightOffset);
				vec::Vec3 pos_up = vec3::VAdd(pos, heightOffset);
				DxlibConverter::DrawLine3D(prevPos_up, pos_up, color);
			}

			prevPos = pos;
			hasPrevPos = true;
		}
		else
		{
			hasPrevPos = false;
		}
	}

	// 中心から両端への線を描画（扇形のエッジ）
	float leftAngle = baseAngle - halfAngleRad;
	float rightAngle = baseAngle + halfAngleRad;

	float sinLeft = sinf(leftAngle);
	float cosLeft = cosf(leftAngle);
	float sinRight = sinf(rightAngle);
	float cosRight = cosf(rightAngle);

	vec::Vec3 leftEdge = vec3::VAdd(center, vec3::VGet(sinLeft * _detectionSector.radius, 0.0f, cosLeft * _detectionSector.radius));
	vec::Vec3 rightEdge = vec3::VAdd(center, vec3::VGet(sinRight * _detectionSector.radius, 0.0f, cosRight * _detectionSector.radius));

	// 床の存在チェックと視線チェック
	bool canDetectLeft = CheckFloorExistence(leftEdge) && CheckLineOfSight(center, leftEdge);
	bool canDetectRight = CheckFloorExistence(rightEdge) && CheckLineOfSight(center, rightEdge);

	if (canDetectLeft)
	{
		DxlibConverter::DrawLine3D(center, leftEdge, color);
		vec::Vec3 center_up = vec3::VAdd(center, heightOffset);
		vec::Vec3 leftEdge_up = vec3::VAdd(leftEdge, heightOffset);
		DxlibConverter::DrawLine3D(center_up, leftEdge_up, color);
	}

	if (canDetectRight)
	{
		DxlibConverter::DrawLine3D(center, rightEdge, color);
		vec::Vec3 center_up = vec3::VAdd(center, heightOffset);
		vec::Vec3 rightEdge_up = vec3::VAdd(rightEdge, heightOffset);
		DxlibConverter::DrawLine3D(center_up, rightEdge_up, color);
	}

	// 敵の正面方向を示す緑の線を描画（敵の位置から索敵中心まで）
	const vec::Vec3 lineOffset = vec3::VGet(0.0f, 5.0f, 0.0f);
	DxlibConverter::DrawLine3D(
		vec3::VAdd(_vPos, lineOffset),
		vec3::VAdd(center, lineOffset),
		GetColor(0, 255, 0)
	);
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