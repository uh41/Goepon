#pragma once
#include "enemybase.h"
#include "playerbase.h"

// 索敵範囲の情報を格納する構造体
struct DetectionSector
{
	VECTOR center;      // 扇形の中心点（敵の位置）
	VECTOR forward;     // 敵の正面方向
	float radius;       // 索敵範囲の半径
	float angle;        // 索敵角度（度）
};

// 検出状態の情報
struct DetectionInfo
{
	bool isDetected;        // 検出されているか
	float timer;            // 検出表示タイマー
	int detectorIndex;      // 検出した敵のインデックス
	VECTOR detectorPos;     // 検出した敵の位置

	// 追加：追跡機能用
	bool isChasing;         // 現在追跡中か
	VECTOR lastKnownPlayerPos; // 最後に確認されたプレイヤーの位置
	float chaseTimer;       // 追跡継続時間
};

class EnemySensor : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// 索敵範囲の設定
	void SetDetectionSector(float radius, float angle);

	// プレイヤーが索敵範囲内にいるかチェック
	bool IsPlayerInDetectionRange(const VECTOR& playerPos) const;

	// プレイヤーの検出チェック（複数のプレイヤーに対応）
	bool CheckPlayerDetection(PlayerBase* player);

	// 検出情報の取得
	const DetectionInfo& GetDetectionInfo() const { return _detectionInfo; }

	// 検出状態のリセット
	void ResetDetection();

	// 追加：追跡機能
	bool IsChasing() const { return _detectionInfo.isChasing; }
	VECTOR GetLastKnownPlayerPosition() const { return _detectionInfo.lastKnownPlayerPos; }
	float GetChaseTimer() const { return _detectionInfo.chaseTimer; }

	// デバッグ用：索敵範囲の描画
	void RenderDetectionSector() const;

	// 検出UI表示
	void RenderDetectionUI() const;

	// センサーの有効/無効状態
	void SetSensorEnabled(bool enabled) { _bSensorEnabled = enabled; }
	bool IsSensorEnabled() const { return _bSensorEnabled; }

protected:
	DetectionSector _detectionSector;  // 索敵範囲
	bool _bHasDetectionSector;         // 索敵範囲が設定されているか
	bool _bSensorEnabled;              // センサーが有効かどうか

	DetectionInfo _detectionInfo;      // 検出状態の情報

	static constexpr float DETECTION_DISPLAY_TIME = 3.0f; // 検出表示時間（秒）

	// 追加：追跡関連定数
	static constexpr float CHASE_TIME = 5.0f; // 追跡継続時間（秒）

	// 内部処理用メソッド
	void UpdateDetectionTimer();       // 検出タイマーの更新
	void UpdateChaseTimer();           // 追加：追跡タイマーの更新
	VECTOR GetDetectionCenter() const; // 索敵範囲の中心位置を取得

};