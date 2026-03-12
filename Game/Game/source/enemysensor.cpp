#include "enemysensor.h"
#include "appframe.h"
#include "map.h"

// 初期化
bool EnemySensor::Initialize()
{
	base::Initialize();

	// 索敵システムの初期化
	_bDetectionSector = false;	
	_bSensorEnabled = true;			

	// 検出情報の初期化
	_detect.bDetected = false;	
	_detect.timer = 0.0f;		

	// 追跡情報の初期化
	_detect.bChasing = false;	
	_detect.lastPlayerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	
	_detect.chaseTimer = 0.0f;	

	// 検知遅延の初期化
	_detect.DelayTimer = 0.0f;
	_detect.bDelay = false;

	// フレームカウンタと検出結果の初期化
	_detectFrameCount = 0;
	_bDetectionResult = false;

	// キャッシュ関連初期化
	_CachedPolygons.clear();
	_CacheCounter = 0;

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

	// 索敵範囲の描画
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
	_detectFrameCount++;
	if (_detectFrameCount < DetectionFrame)
	{
		return _bDetectionResult;
	}
	_detectFrameCount = 0;

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
		if (!_detect.bDetected)
		{
			// 新しく検出された場合
			_detect.bDetected = true;				
			_detect.timer = DETECTION_DISPLAY_TIME;	
		}

		// プレイヤーを検出中は常に位置を更新
		_detect.lastPlayerPos = playerPos;	// 最後に確認された位置更新

		// 追跡開始に遅延
		if (!_detect.bChasing)
		{
			// まだ追跡していない場合
			if (!_detect.bDelay)
			{
				// 遅延開始
				_detect.bDelay = true;
				_detect.DelayTimer = DETECTION_DELAY_TIME;
			}
			else
			{
				// 遅延タイマーを減算
				_detect.DelayTimer -= 1.0f / 60.0f;

				// 遅延時間が経過したら追跡開始
				if (_detect.DelayTimer <= 0.0f)
				{
					_detect.bChasing = true;
					_detect.chaseTimer = CHASE_TIME;
					_detect.bDelay = false;
					_detect.DelayTimer = 0.0f;
				}
			}
		}
		else
		{
			// すでに追跡中の場合は追跡タイマーをリセット
			_detect.chaseTimer = CHASE_TIME;
		}
	}
	else
	{
		// プレイヤーが範囲外に出た場合、遅延状態をリセット
		if (_detect.bDelay)
		{
			_detect.bDelay = false;
			_detect.DelayTimer = 0.0f;
		}
	}

	_bDetectionResult = detected;

	return detected;
}

// カプセル判定版の索敵範囲チェック
bool EnemySensor::IsPlayerInDetectionRangeWithCapsule(
	const vec::Vec3& playerPos,
	const vec::Vec3& playerCapsuleTop,
	const vec::Vec3& playerCapsuleBottom,
	float playerCapsuleRadius
) const
{
	if (!_bDetectionSector || !_bSensorEnabled)
	{
		return false;
	}

	// 索敵範囲の中心位置を取得
	vec::Vec3 detectionCenter = GetDetectionCenter();

	// 扇形の半角をラジアンに変換
	float halfAngleRad = (_detectSector.angle * 0.5f) * DX_PI_F / 180.0f;

	// 正規化された前方ベクトル
	vec::Vec3 forwardNorm = vec3::VNorm(_vDir);

	// CollisionManagerのCheckSectorToCapsuleを使用
	bool detected = CollisionManager::GetInstance()->CheckSectorToCapsule(
		detectionCenter,
		forwardNorm,
		_detectSector.radius,
		halfAngleRad,
		playerCapsuleTop,
		playerCapsuleBottom,
		playerCapsuleRadius
	);

	if (!detected)
	{
		return false;
	}

	// 敵の位置からプレイヤーの位置まで床の存在を一定間隔でチェック
	if (!CheckLineOfSight(detectionCenter, playerPos))
	{
		return false; // 視線が遮断されている
	}

	return true;
}

// 検出状態のリセット
void EnemySensor::ResetDetection()
{
	// 検出状態リセット
	_detect.bDetected = false;	
	_detect.timer = 0.0f;		

	// 追跡状態リセット
	_detect.bChasing = false;	
	_detect.lastPlayerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	
	_detect.chaseTimer = 0.0f;	

	// 検知遅延のリセット
	_detect.bDelay = false;
	_detect.DelayTimer = 0.0f;

	// フレームカウンタと検出結果のリセット
	_detectFrameCount = 0;
	_bDetectionResult = false;
}

// 検出タイマーの更新
void EnemySensor::UpdateDetectionTimer()
{
	// 追跡タイマーの更新
	if (_detect.bChasing)
	{
		_detect.chaseTimer -= 1.0f / 60.0f; // 60FPSとして計算

		if (_detect.chaseTimer <= 0.0f)
		{
			_detect.bChasing = false;
			_detect.chaseTimer = 0.0f;
		}
	}

	// 検出表示タイマーの更新
	if (_detect.timer > 0.0f)
	{
		_detect.timer -= 1.0f / 60.0f; 

		if (_detect.timer <= 0.0f)
		{
			_detect.bDetected = false;
		}
	}
}

// 索敵範囲の設定
void EnemySensor::SetDetectionSector(float radius, float angle)
{
	_detectSector.radius = radius;
	_detectSector.angle = angle;
	_bDetectionSector = true;
}

// 索敵範囲の中心位置を取得（敵の正面に配置）
vec::Vec3 EnemySensor::GetDetectionCenter() const
{
	// 敵の正面方向に一定距離進んだ位置を中心にする
	const float offsetDistance = _detectSector.radius * 0.1f; // 半径の10%前方
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
	const float checkInterval = 10.0f;

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

// 検出UI表示
void EnemySensor::RenderDetectionUI() const
{
	if (_detect.bDetected && _detect.timer > 0.0f)
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
			"Timer: %.1f", _detect.timer);
	}
}

// キャッシュを再計算する実装
void EnemySensor::RecalculateDetectionSector() const
{
	// ★ 静的なグローバルカウンタ：全敵で共有
	static int s_globalUpdateFrame = 0;

	// ★ 今回の敵が更新対象かどうかを判定
	// _enemyId を使って「このフレームで自分の番か？」をチェック
	if (s_globalUpdateFrame % CacheInterval != (_enemyId % CacheInterval))
	{
		// まだこの敵の更新タイミングではない → キャッシュ再計算をスキップ
		return;
	}

	_CachedPolygons.clear();

	// 索敵範囲が設定されていない場合は何もしない
	if (!_bDetectionSector)
	{
		return;
	}

	// 色（検出時 / 非検出時）
	COLOR_U8 fillColorU8;
	if (_detect.bChasing)
	{
		// 追跡中：赤色
		fillColorU8 = GetColorU8(255, 0, 0, 255);
	}
	else if (_detect.bDetected || _soundDetectionActive || _isMovingToSound)
	{
		// 検知中（追跡前）：黄色
		// 音検知や音源へ移動中も黄色にする
		fillColorU8 = GetColorU8(255, 255, 0, 255);
	}
	else
	{
		// 通常時：水色
		fillColorU8 = GetColorU8(0, 255, 255, 255);
	}

	const int angleSegments = 16;	// 扇形の角度を分割する数
	const int radiusSegments = 7;	// 半径方向の分割数
	const float halfAngleRad = (_detectSector.angle * 0.5f) * DX_PI_F / 180.0f;
	const vec::Vec3 center = GetDetectionCenter();
	const vec::Vec3 forward = vec3::VNorm(_vDir);
	const float baseAngle = atan2f(forward.x, forward.z);
	const vec::Vec3 heightOffset = vec3::VGet(0.0f, 10.0f, 0.0f);

	// 頂点初期化ラムダ
	auto initVertex = [&](VERTEX3D& v, const vec3::Vec3& p)
		{
			v.pos = VGet(p.x, p.y, p.z);
			v.norm = VGet(0.0f, 1.0f, 0.0f);
			v.dif = fillColorU8;
			v.spc = GetColorU8(0, 0, 0, 0);
			v.u = v.v = v.su = v.sv = 0.0f;
		};

	VERTEX3D verts[6];
	for (int r = 0; r < radiusSegments; ++r)
	{
		const float innerRadius = (_detectSector.radius / radiusSegments) * r;
		const float outerRadius = (_detectSector.radius / radiusSegments) * (r + 1);

		vec::Vec3 prevInnerPos{};
		vec::Vec3 prevOuterPos{};
		bool hasPrev = false;

		for (int i = 0; i <= angleSegments; ++i)
		{
			const float angle = baseAngle + (-halfAngleRad + (2.0f * halfAngleRad * i / StCas<float>(angleSegments)));
			const float s = sinf(angle);
			const float c = cosf(angle);

			// 元の平面XYでの位置（yは一旦 center.y を基準にしておく）
			vec::Vec3 innerPos = vec3::VAdd(center, vec3::VGet(s * innerRadius, 0.0f, c * innerRadius));
			vec::Vec3 outerPos = vec3::VAdd(center, vec3::VGet(s * outerRadius, 0.0f, c * outerRadius));

			// 安定した床判定のため、各サンプルの y を床の Y に合わせる。
			constexpr float kFloorSearchColSubY = 200.0f;
			float floorYInner = 0.0f;
			float floorYOuter = 0.0f;
			bool hasFloorInner = GetFloorYCollision(innerPos, kFloorSearchColSubY, floorYInner);
			bool hasFloorOuter = GetFloorYCollision(outerPos, kFloorSearchColSubY, floorYOuter);

			// 床が見つかった場合はその高さに合わせる（若干オフセットしたい場合は +eps を加算）
			if (hasFloorInner)
			{
				innerPos.y = floorYInner;
			}
			if (hasFloorOuter)
			{
				outerPos.y = floorYOuter;
			}

			// 床があり、かつ中心からの視線が通っている場合のみ有効とする
			const bool canDetect = hasFloorOuter && CheckLineOfSight(center, outerPos);

			if (canDetect && hasPrev)
			{
				// 1つ目の三角形: prevInner, prevOuter, innerPos
				initVertex(verts[0], prevInnerPos);
				initVertex(verts[1], prevOuterPos);
				initVertex(verts[2], innerPos);

				// 2つ目の三角形: prevOuter, outerPos, innerPos
				initVertex(verts[3], prevOuterPos);
				initVertex(verts[4], outerPos);
				initVertex(verts[5], innerPos);

				// 少し上にも描画する分もキャッシュ（視認性向上）
				for (int vi = 0; vi < 6; ++vi)
				{
					verts[vi].pos = VGet(verts[vi].pos.x, verts[vi].pos.y + heightOffset.y, verts[vi].pos.z);
				}
				std::array<VERTEX3D, 6> polyUp{};
				for (int vi = 0; vi < 6; ++vi) polyUp[vi] = verts[vi];
				_CachedPolygons.push_back(polyUp);
			}

			prevInnerPos = innerPos;
			prevOuterPos = outerPos;
			hasPrev = canDetect;
		}
	}
}

// 描画時はキャッシュを使う
void EnemySensor::RenderDetectionSector() const
{
	// 索敵範囲が設定されていない場合は描画しない
	if (!_bDetectionSector)
	{
		return;
	}

	// ★ 静的なグローバルカウンタをインクリメント（全敵で共有）
	static int s_globalUpdateFrame = 0;
	s_globalUpdateFrame++;

	// キャッシュの更新タイミング制御
	if (_CacheCounter <= 0 || _CachedPolygons.empty())
	{
		RecalculateDetectionSector();
		_CacheCounter = CacheInterval;
	}
	// 次回までカウントダウン
	_CacheCounter--;

	// 描画設定
	SetUseLighting(FALSE);	// ライティング無効
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 78); // 半透明

	// キャッシュから描画（各要素は 2 ポリゴン分の頂点：配列サイズ 6）
	for (const auto& poly : _CachedPolygons)
	{
		DrawPolygon3D(poly.data(), 2, DX_NONE_GRAPH, FALSE);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetUseLighting(TRUE);
}