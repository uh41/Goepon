#pragma once
#include "enemybase.h"
#include "playerbase.h"
#include "MapBase.h"

// 敵の索敵範囲を表す構造体
struct DetectionSector
{
	vec::Vec3 center;   // 中心位置
	vec::Vec3 forward;  // 前方ベクトル（正規化されていることが前提）
	float radius;       // 半径
	float angle;		// 視野角（度数法）
};

// プレイヤーの検出情報を表す構造体
struct DetectionInfo
{
	bool bDetected;				// プレイヤーが検出されたかどうか
	bool bChasing;				// 追跡状態かどうか
	bool bDelay;				// 遅延中フラグ
	vec::Vec3 lastPlayerPos;	// 最後に検出されたプレイヤーの位置
	float timer;				// プレイヤーが検出されてからの経過時間
	float chaseTimer;			// 追跡状態の経過時間
	float DelayTimer;			// 検知してから追跡開始までの遅延タイマー
};

class EnemySensor : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// 索敵範囲を設定する関数
	void SetDetectionSector(float radius, float angle);	

	// プレイヤーを検出する関数
	bool CheckPlayerDetection(PlayerBase* player);

	// 索敵情報のゲッター
	const DetectionInfo& GetDetectionInfo() const { return _detect; }

	// 索敵状態のリセット
	void ResetDetection();

	// 追跡状態のセッターとゲッター
	bool IsChasing() const { return _detect.bChasing; } 
	vec::Vec3 GetLastPlayerPos() const { return _detect.lastPlayerPos; }
	float GetChaseTimer() const { return _detect.chaseTimer; }

	// 索敵範囲表示
	void RenderDetectionSector() const;

	// デバッグ用の索敵結果表示
	void RenderDetectionUI() const;

	// センサーの有効/無効を切り替えるセッターとゲッター
	void SetSensorEnabled(bool enabled) { _bSensorEnabled = enabled; }
	bool IsSensorEnabled() const { return _bSensorEnabled; }

	// マップ情報を設定するためのセッター
	void SetMap(MapBase* map) { _map = map; }

	// 指定した位置に床が存在するか
	bool CheckFloorExistence(const vec::Vec3& position) const;

	// 指定した位置からプレイヤーまでの間の障害物をチェック
	bool CheckLineOfSight(const vec::Vec3& startPos, const vec::Vec3& endPos) const;

	// 指定した位置から床のY座標を取得
	bool GetFloorYCollision(const vec::Vec3& position, float colSubY, float& outY) const;

	// プレイヤーの位置とカプセル形状を考慮して、索敵範囲内にいるかどうかを判定する
	bool IsPlayerInDetectionRangeWithCapsule(
		const vec::Vec3& playerPos,
		const vec::Vec3& playerCapsuleTop,
		const vec::Vec3& playerCapsuleBottom,
		float playerCapsuleRadius
	) const;

protected:
	DetectionSector _detectSector;	// 索敵範囲の情報
	DetectionInfo _detect;			// プレイヤーの検出情報

	bool _bDetectionSector;			// 索敵範囲が設定されているかどうかのフラグ
	bool _bSensorEnabled;			// センサーが有効かどうかのフラグ   

	// 索敵結果の表示時間（秒）
	static constexpr float DETECTION_DISPLAY_TIME = 0.1f;

	// 追跡状態の持続時間（秒）
	static constexpr float CHASE_TIME = 5.0f;

	int _detectFrameCount;
	static constexpr int DetectionFrame = 1;	
	bool _bDetectionResult;					

	// 索敵範囲の描画用キャッシュとタイマー
	void UpdateDetectionTimer();		  
	vec::Vec3 GetDetectionCenter() const; 

	MapBase* _map;

	// 描画キャッシュ（const メソッドから更新するため mutable）
	mutable std::vector<std::array<VERTEX3D, 6>> _CachedPolygons;
	mutable int _CacheCounter;

	// キャッシュ再計算インターバル（フレーム）
	static constexpr int CacheInterval = 5;

	// キャッシュを再計算する（描画用。内部で重い判定を実行）
	void RecalculateDetectionSector() const;

	// プレイヤーを検出してから追跡開始までの遅延時間（秒）
	static constexpr float DETECTION_DELAY_TIME = 0.35f;
};